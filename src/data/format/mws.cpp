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


// ----------------------------------------------------------------------------- : Utilities

/// Convert a tagged string to MWS format: \t\t before each line beyond the first
String untag_mws(const String& str) {
	// TODO : em dashes?
	return replace_all(untag(curly_quotes(str,false)), (L"\n"), (L"\n\t\t") );
}
inline String untag_mws(const ScriptValueP& str) {
	return untag_mws(str->toString());
}
//String untag_mws(const Defaultable<String>& str) {
//	str.
//}

/// Code for card color in MWS format
String card_color_mws(const String& col) {
	if (col == (L"white"))     return (L"W");
	if (col == (L"blue"))      return (L"U");
	if (col == (L"black"))     return (L"B");
	if (col == (L"red"))       return (L"R");
	if (col == (L"green"))     return (L"G");
	if (col == (L"artifact"))  return (L"Art");
	if (col == (L"colorless")) return (L"Art");
	if (col.find((L"land")) != String::npos) {
		return (L"Lnd"); // land
	} else {
		return (L"Gld"); // multicolor
	}
}

/// Code for card rarity, used for MWS and Apprentice
String card_rarity_code(const String& rarity) {
	if (rarity == (L"rare"))     return (L"R");
	if (rarity == (L"uncommon")) return (L"U");
	else                         return (L"C");
}

// ----------------------------------------------------------------------------- : export_mws

void export_mws(Window* parent, const SetP& set) {
	if (!set->game->isMagic()) {
		throw Error((L"Can only export Magic sets to Magic Workstation"));
	}
	
	// Select filename
	String name = wxFileSelector((L"Export to file"),settings.default_export_dir,(L""),(L""),
		                         (L"Text files (*.txt)|*.txt|All Files|*"),
		                         wxFD_SAVE | wxFD_OVERWRITE_PROMPT, parent);
	if (name.empty()) return;
	settings.default_export_dir = wxPathOnly(name);
	wxBusyCursor busy;
	// Open file
	wxFileOutputStream f(name);
	wxTextOutputStream file(f, wxEOL_DOS);
	
	// Write header
	file.WriteString(set->value((L"title"))->toString() + (L" Spoiler List\n"));
	file.WriteString((L"Set exported using Magic Set Editor 2, version ") + app_version.toString() + (L"\n\n"));
	wxDateTime now = wxDateTime::Now();
	file.WriteString((L"Spoiler List created on ") + now.FormatISODate() + (L" ") + now.FormatISOTime());
	file.WriteString((L"\n\n"));
	
	// Write cards
	for(auto& card : set->cards) {
		file.WriteString((L"Card Name:\t"));
		file.WriteString(untag_mws(card->value((L"name"))));
		file.WriteString((L"\nCard Color:\t"));
		file.WriteString(card_color_mws(card->value((L"card color"))->toString()));
		file.WriteString((L"\nMana Cost:\t"));
		file.WriteString(untag_mws(card->value((L"casting cost"))));
		file.WriteString((L"\nType & Class:\t"));
		String sup_type = untag_mws(card->value((L"super type")));
		String sub_type = untag_mws(card->value((L"sub type")));
		if (sub_type.empty()) {
			file.WriteString(sup_type);
		} else {
			file.WriteString(sup_type + (L" - ") + sub_type);
		}
		file.WriteString((L"\nPow/Tou:\t"));
		file.WriteString(untag_mws(card->value((L"pt"))));
		file.WriteString((L"\nCard Text:\t"));
		file.WriteString(untag_mws(card->value((L"rule text"))));
		file.WriteString((L"\nFlavor Text:\t"));
		file.WriteString(untag_mws(card->value((L"flavor text"))));
		file.WriteString((L"\nArtist:\t\t"));
		file.WriteString(untag_mws(card->value((L"illustrator"))));
		file.WriteString((L"\nRarity:\t\t"));
		file.WriteString(card_rarity_code(card->value((L"rarity"))->toString()));
		file.WriteString((L"\nCard #:\t\t"));
		file.WriteString(untag_mws(card->value((L"card number"))));
		file.WriteString((L"\n\n"));
	}
}
