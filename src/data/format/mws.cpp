//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/format/formats.hpp>
#include <data/game.hpp>
#include <data/set.hpp>
#include <data/card.hpp>
#include <data/field/text.hpp>
#include <data/field/choice.hpp>
#include <util/string.hpp>
#include <util/tagged_string.hpp>
#include <wx/wfstream.h>

DECLARE_TYPEOF_COLLECTION(CardP);

// ----------------------------------------------------------------------------- : Utilities

/// Convert a tagged string to MWS format: \t\t before each line beyond the first
String untag_mws(const String& str) {
	// TODO : em dashes?
	return replace_all(untag(curly_quotes(str,false)), _("\n"), _("\n\t\t") );
}
inline String untag_mws(const ScriptValueP& str) {
	return untag_mws(str->toString());
}
//String untag_mws(const Defaultable<String>& str) {
//	str.
//}

/// Code for card color in MWS format
String card_color_mws(const String& col) {
	if (col == _("white"))     return _("W");
	if (col == _("blue"))      return _("U");
	if (col == _("black"))     return _("B");
	if (col == _("red"))       return _("R");
	if (col == _("green"))     return _("G");
	if (col == _("artifact"))  return _("Art");
	if (col == _("colorless")) return _("Art");
	if (col.find(_("land")) != String::npos) {
		return _("Lnd"); // land
	} else {
		return _("Gld"); // multicolor
	}
}

/// Code for card rarity, used for MWS and Apprentice
String card_rarity_code(const String& rarity) {
	if (rarity == _("rare"))     return _("R");
	if (rarity == _("uncommon")) return _("U");
	else                         return _("C");
}

// ----------------------------------------------------------------------------- : export_mws

void export_mws(Window* parent, const SetP& set) {
	if (!set->game->isMagic()) {
		throw Error(_("Can only export Magic sets to Magic Workstation"));
	}
	
	// Select filename
	String name = wxFileSelector(_("Export to file"),settings.default_export_dir,_(""),_(""),
		                         _("Text files (*.txt)|*.txt|All Files|*"),
		                         wxFD_SAVE | wxFD_OVERWRITE_PROMPT, parent);
	if (name.empty()) return;
	settings.default_export_dir = wxPathOnly(name);
	wxBusyCursor busy;
	// Open file
	wxFileOutputStream f(name);
	wxTextOutputStream file(f, wxEOL_DOS);
	
	// Write header
	file.WriteString(set->value(_("title"))->toString() + _(" Spoiler List\n"));
	file.WriteString(_("Set exported using Magic Set Editor 2, version ") + app_version.toString() + _("\n\n"));
	wxDateTime now = wxDateTime::Now();
	file.WriteString(_("Spoiler List created on ") + now.FormatISODate() + _(" ") + now.FormatISOTime());
	file.WriteString(_("\n\n"));
	
	// Write cards
	FOR_EACH(card, set->cards) {
		file.WriteString(_("Card Name:\t"));
		file.WriteString(untag_mws(card->value(_("name"))));
		file.WriteString(_("\nCard Color:\t"));
		file.WriteString(card_color_mws(card->value(_("card color"))->toString()));
		file.WriteString(_("\nMana Cost:\t"));
		file.WriteString(untag_mws(card->value(_("casting cost"))));
		file.WriteString(_("\nType & Class:\t"));
		String sup_type = untag_mws(card->value(_("super type")));
		String sub_type = untag_mws(card->value(_("sub type")));
		if (sub_type.empty()) {
			file.WriteString(sup_type);
		} else {
			file.WriteString(sup_type + _(" - ") + sub_type);
		}
		file.WriteString(_("\nPow/Tou:\t"));
		file.WriteString(untag_mws(card->value(_("pt"))));
		file.WriteString(_("\nCard Text:\t"));
		file.WriteString(untag_mws(card->value(_("rule text"))));
		file.WriteString(_("\nFlavor Text:\t"));
		file.WriteString(untag_mws(card->value(_("flavor text"))));
		file.WriteString(_("\nArtist:\t\t"));
		file.WriteString(untag_mws(card->value(_("illustrator"))));
		file.WriteString(_("\nRarity:\t\t"));
		file.WriteString(card_rarity_code(card->value(_("rarity"))->toString()));
		file.WriteString(_("\nCard #:\t\t"));
		file.WriteString(untag_mws(card->value(_("card number"))));
		file.WriteString(_("\n\n"));
	}
}
