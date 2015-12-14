//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// -----------------------------------------------------------------------------
// : Includes

#include <util/prec.hpp>
#include <data/format/formats.hpp>
#include <data/set.hpp>
#include <data/game.hpp>
#include <data/stylesheet.hpp>
#include <data/card.hpp>
#include <data/field/text.hpp>
#include <data/field/choice.hpp>
#include <data/field/image.hpp>
#include <util/tagged_string.hpp>
#include <wx/wfstream.h>

ScriptValueP script_local_image_file(LocalFileName const &filename);

// -----------------------------------------------------------------------------
// : MtgEditorFileFormat

/// The file format of Mtg Editor files
class MtgEditorFileFormat : public FileFormat {
  public:
    virtual String extension() { return (L"set"); }
    virtual String name() { return (L"Mtg Editor files (*.set)"); }
    virtual bool canImport() { return true; }
    virtual bool canExport(const Game &) { return false; }
    virtual SetP importSet(const String &filename);

  private:
    // Filter: set filename -> image directory
    // based on MtgEditor's: CardSet.getImageFolder
    String filter1(const String &str);
    // Filter: card name -> image name
    // based on MtgEditor's: Tools::purgeSpecialChars
    String filter2(const String &str);
    // Remove mtg editor tags
    void untag(const CardP &card, const String &field);
    // Translate all tags, mana tags get converted to <sym>, other tags are
    // removed
    void translateTags(String &value);
    void translateTags(ScriptValueP &value);
};

FileFormatP mtg_editor_file_format() {
    return intrusive(new MtgEditorFileFormat());
}

// -----------------------------------------------------------------------------
// : Importing

void append_line(ScriptValueP &target, String const &line);

SetP MtgEditorFileFormat::importSet(const String &filename) {
    wxFileInputStream f(filename);
    wxTextInputStream file(f, (L'\n'), wxConvLibc);
    // create set
    SetP set(new Set(Game::byName((L"magic"))));

    // parsing state
    CardP current_card;
    ScriptValueP *target = nullptr; // value we are currently reading
    String layout = (L"8e");
    String set_date, card_date;
    bool first = true;

    // read file
    while (!f.Eof()) {
        // read a line
        if (!current_card)
            current_card = intrusive(new Card(*set->game));
        String line = file.ReadLine();
        if (line == (L"#SET###########")) { // set.title
            target = &set->value((L"title"));
        } else if (line == (L"#SETDATE#######")) { // date
            // remember date for generation of illustration filename
            target = nullptr;
            set_date = file.ReadLine();
        } else if (line == (L"#SHOWRARITY####") ||
                   line == (L"#FONTS#########") ||
                   line == (L"#COUNT#########")) { // ignore
            target = nullptr;
            file.ReadLine();
        } else if (line == (L"#LAYOUT########")) { // layout type
            target = nullptr;
            layout = file.ReadLine();
        } else if (line == (L"#CARD##########") ||
                   line == (L"#EOF###########")) { // begin/end of card
            if (!first) {
                // First [CARD##] indicates only the start of a card, subsequent
                // ones also the end of the previous
                // card. We only care about the latter
                // remove all tags from all text values
                untag(current_card, (L"name"));
                untag(current_card, (L"super type"));
                untag(current_card, (L"sub type"));
                untag(current_card, (L"casting cost"));
                untag(current_card, (L"flavor text"));
                untag(current_card, (L"illustrator"));
                untag(current_card, (L"copyright"));
                untag(current_card, (L"power"));
                untag(current_card, (L"toughness"));
                // translate mtg editor tags to mse2 tags
                translateTags(current_card->value((L"rule text")));
                // add the card to the set
                set->cards.push_back(current_card);
            }
            first = false;
            current_card = intrusive(new Card(*set->game));
            target = &current_card->value((L"name"));
        } else if (line == (L"#DATE##########")) { // date
            // remember date for generation of illustration filename
            target = nullptr;
            card_date = file.ReadLine();
        } else if (line == (L"#TYPE##########")) { // super type
            target = &current_card->value((L"super type"));
        } else if (line == (L"#SUBTYPE#######")) { // sub type
            target = &current_card->value((L"sub type"));
        } else if (line == (L"#COST##########")) { // casting cost
            target = &current_card->value((L"casting cost"));
        } else if (line == (L"#RARITY########") ||
                   line == (L"#FREQUENCY#####")) { // rarity
            target = 0;
            line = file.ReadLine();
            if (line == (L"0"))
                current_card->value((L"rarity")) = to_script((L"common"));
            else if (line == (L"1"))
                current_card->value((L"rarity")) = to_script((L"uncommon"));
            else
                current_card->value((L"rarity")) = to_script((L"rare"));
        } else if (line == (L"#COLOR#########")) { // card color
            target = 0;
            line = file.ReadLine();
            current_card->value((L"card color")) = to_script(line);
        } else if (line == (L"#AUTOBG########")) { // card color.isDefault
            target = 0;
            line = file.ReadLine();
            if (line == (L"TRUE")) {
                current_card->value((L"card color")) = script_default_nil;
            }
        } else if (line == (L"#RULETEXT######")) { // rule text
            target = &current_card->value((L"rule text"));
        } else if (line == (L"#FLAVORTEXT####")) { // flavor text
            target = &current_card->value((L"flavor text"));
        } else if (line == (L"#ARTIST########")) { // illustrator
            target = &current_card->value((L"illustrator"));
        } else if (line == (L"#COPYRIGHT#####")) { // copyright
            target = &current_card->value((L"copyright"));
        } else if (line == (L"#POWER#########")) { // power
            target = &current_card->value((L"power"));
        } else if (line == (L"#TOUGHNESS#####")) { // toughness
            target = &current_card->value((L"toughness"));
        } else if (line == (L"#ILLUSTRATION##") ||
                   line == (L"#ILLUSTRATION8#")) { // image
            target = 0;
            line = file.ReadLine();
            if (!wxFileExists(line)) {
                // based on card name and date
                line = filter1(filename) + set_date + (L"/") +
                       filter2(current_card->value((L"name"))->toString()) +
                       card_date + (L".jpg");
            }
            // copy image into set
            if (wxFileExists(line)) {
                FileName image_file = set->newFileName((L"image"), (L""));
                if (wxCopyFile(line, set->nameOut(image_file), true)) {
                    current_card->value((L"image")) =
                        script_local_image_file(image_file);
                }
            }
        } else if (line == (L"#TOMBSTONE#####")) { // tombstone
            target = 0;
            line = file.ReadLine();
            current_card->value((L"card symbol")) =
                to_script(line == (L"TRUE") ? (L"tombstone") : (L"none"));
        } else {
            // normal text
            if (target != 0) { // value of a text field
                append_line(*target, line);
            } else {
                throw ParseError(
                    (L"Error in Mtg Editor file, unexpected text:\n") + line);
            }
        }
    }

    // set defaults for artist and copyright to that of the first card
    if (!set->cards.empty()) {
        ScriptValueP &artist = set->cards[0]->value((L"illustrator"));
        ScriptValueP &copyright = set->cards[0]->value((L"copyright"));
        set->value((L"artist")) = artist;
        set->value((L"copyright")) = copyright;
        // which cards have this value?
        for (auto &card : set->cards) {
            ScriptValueP &card_artist = card->value((L"illustrator"));
            ScriptValueP &card_copyright = card->value((L"copyright"));
            if (equal(card_artist, artist))
                card_artist = make_default(artist);
            if (equal(card_copyright, copyright))
                card_copyright = make_default(card_copyright);
        }
    }

    // Load stylesheet
    if (layout != (L"8e")) {
        try {
            set->stylesheet = StyleSheet::byGameAndName(*set->game, (L"old"));
        } catch (const Error &) {
            // If old style doesn't work try the new one
            set->stylesheet = StyleSheet::byGameAndName(*set->game, (L"new"));
        }
    } else {
        set->stylesheet = StyleSheet::byGameAndName(*set->game, (L"new"));
    }

    set->validate();
    return set;
}

// -----------------------------------------------------------------------------
// : Filtering

String MtgEditorFileFormat::filter1(const String &str) {
    String before, after, ret;
    // split path name
    size_t pos = str.find_last_of((L"/\\"));
    if (pos == String::npos) {
        before = (L"");
        after = str;
    } else {
        before = str.substr(0, pos + 1);
        after = str.substr(pos + 1);
    }
    // filter
    for (auto &c : after) {
        if (isAlnum(c))
            ret += c;
        else
            ret += (L'_');
    }
    return before + ret;
}

String MtgEditorFileFormat::filter2(const String &str) {
    String ret;
    for (const auto &c : str) {
        if (isAlnum(c))
            ret += c;
        else if (c == (L' ') || c == (L'-'))
            ret += (L'_');
    }
    return ret;
}

void MtgEditorFileFormat::untag(const CardP &card, const String &field) {
    ScriptValueP &value = card->value(field);
    value = with_defaultness_of(value,
                                to_script(untag_no_escape(value->toString())));
}

void MtgEditorFileFormat::translateTags(String &value) {
    // Translate tags
    String ret;
    size_t pos = 0;
    while (pos < value.size()) {
        Char c = value.GetChar(pos);
        ++pos;
        if (c == (L'<')) {
            String tag;
            while (pos < value.size()) {
                c = value.GetChar(pos);
                ++pos;
                if (c == (L'>'))
                    break;
                else
                    tag += c;
            }
            tag.MakeUpper();
            unsigned long number;
            if (tag == (L"W") || tag == (L"U") || tag == (L"B") ||
                tag == (L"R") || tag == (L"G") || tag == (L"X") ||
                tag == (L"Y") || tag == (L"Z")) {
                ret += (L"<sym>") + tag + (L"</sym>");
            } else if (tag.ToULong(&number)) {
                ret += (L"<sym>") + tag + (L"</sym>");
            } else if (tag == (L"T") || tag == (L"TAP")) {
                ret += (L"<sym>T</sym>");
            } else if (tag == (L"THIS")) {
                ret += (L"~");
            }
        } else {
            ret += c;
        }
    }
    // Join adjecent symbol sections
    value = replace_all(ret, (L"</sym><sym>"), (L""));
}
void MtgEditorFileFormat::translateTags(ScriptValueP &value) {
    String val = value->toString();
    translateTags(val);
    value = to_script(val);
}
