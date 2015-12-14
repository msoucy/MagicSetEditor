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
#include <wx/wfstream.h>

ScriptValueP script_local_image_file(LocalFileName const &filename);

// -----------------------------------------------------------------------------
// : MSE1FileFormat

/// The file format of MSE1 files
class MSE1FileFormat : public FileFormat {
  public:
    virtual String extension() { return (L"mse"); }
    virtual String name() {
        return (L"Magic Set Editor version 1 files (*.mse)");
    }
    virtual bool canImport() { return true; }
    virtual bool canExport(const Game &) { return false; }
    virtual SetP importSet(const String &filename);
};

FileFormatP mse1_file_format() { return intrusive(new MSE1FileFormat()); }

// -----------------------------------------------------------------------------
// : Importing

// read a card from a mse1 file, add to the set when done
void read_mse1_card(Set &set, wxFileInputStream &f, wxTextInputStream &file);

SetP MSE1FileFormat::importSet(const String &filename) {
    wxFileInputStream f(filename);
    wxTextInputStream file(f, (L'\n'), wxConvLibc);
    // create set
    SetP set(new Set(Game::byName((L"magic"))));

    // file version check
    String format = file.ReadLine();
    if (format.substr(0, 8) != (L"MTG Set8")) {
        throw ParseError(
            (L"Expected MSE format version 8\nTo convert files made with older "
             L"versions of Magic Set Editor:\n  1. Download the latest version "
             L"1 from http:://magicsetedtitor.sourceforge.net\n  2. Open the "
             L"set, then save the set\n  3. Try to open them again in this "
             L"program."));
    }
    // read general info
    set->value((L"title")) = to_script(file.ReadLine());
    set->value((L"artist")) = to_script(file.ReadLine());
    set->value((L"copyright")) = to_script(file.ReadLine());
    file.ReadLine(); // border color, ignored
    String stylesheet = file.ReadLine();
    set->apprentice_code = file.ReadLine(); // apprentice prefix
    file.ReadLine(); // 'formatN'?, not even used by MSE1 :S, ignored
    file.ReadLine(); // 'formatS'?, same, ignored
    file.ReadLine(); // symbol filename, ignored
    file.ReadLine(); // use black symbol for all rarities, ignored
    String desc, line;
    while (!f.Eof()) {
        line = file.ReadLine();
        if (line == (L"\xFF"))
            break;
        desc += line;
    }
    set->value((L"description")) = to_script(desc);

    // load stylesheet
    if (stylesheet.substr(0, 3) == (L"old")) {
        try {
            set->stylesheet = StyleSheet::byGameAndName(*set->game, (L"old"));
        } catch (const Error &) {
            // If old style doesn't work try the new one
            set->stylesheet = StyleSheet::byGameAndName(*set->game, (L"new"));
        }
    } else {
        set->stylesheet = StyleSheet::byGameAndName(*set->game, (L"new"));
    }

    // read cards
    CardP current_card;
    while (!f.Eof()) {
        read_mse1_card(*set, f, file);
    }

    // done
    set->validate();
    return set;
}

// append a line to a ScriptString, this is a bit inefficient, since we keep
// copying the string
void append_line(ScriptValueP &target, String const &line) {
    String old_value = target->toString();
    if (!is_default(target))
        old_value += (L"\n");
    target = to_script(old_value + line);
}

void read_mse1_card(Set &set, wxFileInputStream &f, wxTextInputStream &file) {
    CardP card(new Card(*set.game));
    while (!f.Eof()) {
        // read a line
        String line = file.ReadLine();
        if (line.empty())
            continue;
        Char type = line.GetChar(0);
        line = line.substr(1);
        // interpret this line
        switch (type) {
        case 'A': { // done
            set.cards.push_back(card);
            return;
        }
        case 'B': { // name
            card->value((L"name")) = to_script(line);
            break;
        }
        case 'C':
        case 'D': { // image filename
            LocalFileName image_file = set.newFileName(
                (L"image"), (L"")); // a new unique name in the package
            if (wxCopyFile(line, set.nameOut(image_file), true)) {
                card->value((L"image")) = script_local_image_file(image_file);
            }
            break;
        }
        case 'E': { // super type
            card->value((L"super type")) = to_script(line);
            break;
        }
        case 'F': { // sub type
            card->value((L"sub type")) = to_script(line);
            break;
        }
        case 'G': { // casting cost
            card->value((L"casting cost")) = to_script(line);
            break;
        }
        case 'H': { // rarity
            String rarity;
            if (line == (L"(U)"))
                rarity = (L"uncommon");
            else if (line == (L"(R)"))
                rarity = (L"rare");
            else
                rarity = (L"common");
            card->value((L"rarity")) = to_script(rarity);
            break;
        }
        case 'I': { // power/thoughness
            size_t pos = line.find_first_of((L'/'));
            if (pos != String::npos) {
                card->value((L"power")) = to_script(line.substr(0, pos));
                card->value((L"toughness")) = to_script(line.substr(pos + 1));
            }
            break;
        }
        case 'J': { // rule text or part of text
            append_line(card->value((L"rule text")), line);
            break;
        }
        case 'K': { // flavor text or part of text
            append_line(card->value((L"flavor text")), line);
            break;
        }
        case 'L': { // card color (if not default)
            // decode color
            String color;
            if (line == (L"1"))
                color = (L"white");
            else if (line == (L"2"))
                color = (L"blue");
            else if (line == (L"3"))
                color = (L"black");
            else if (line == (L"4"))
                color = (L"red");
            else if (line == (L"5"))
                color = (L"green");
            else if (line == (L"6"))
                color = (L"colorless");
            else if (line == (L"7"))
                color = (L"land");
            else if (line == (L"9"))
                color = (L"multicolor");
            else
                color = (L"colorless");
            card->value((L"card color")) = to_script(color);
            break;
        }
        default: { throw ParseError((L"Not a valid MSE1 file")); }
        }
    }
}
