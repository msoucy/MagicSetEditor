//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FIELD_PACKAGE_CHOICE
#define HEADER_DATA_FIELD_PACKAGE_CHOICE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field.hpp>
#include <data/font.hpp>
#include <script/scriptable.hpp>

DECLARE_POINTER_TYPE(Packaged);

// ----------------------------------------------------------------------------- : PackageChoiceField

DECLARE_POINTER_TYPE(PackageChoiceField);
DECLARE_POINTER_TYPE(PackageChoiceStyle);

/// A field for PackageChoice values, it contains a list of choices for PackageChoices
class PackageChoiceField : public Field {
  public:
	PackageChoiceField() : required(true), empty_name(_("none")) {}
	DECLARE_FIELD_TYPE();
	
	String             match;			///< Glob of package filenames to match
	bool               required;		///< Is selecting a package required?
	String             empty_name;		///< Displayed name for the empty value (if !required)
};

// ----------------------------------------------------------------------------- : PackageChoiceStyle

/// The Style for a PackageChoiceField
class PackageChoiceStyle : public Style {
  public:
	PackageChoiceStyle(const PackageChoiceFieldP& field);
	DECLARE_STYLE_TYPE(PackageChoice);
	
	Font font;	///< Font to use for the text
	
	virtual int update(Context&);
};

// ----------------------------------------------------------------------------- : PackageChoiceValue

typedef Value PackageChoiceValue;
typedef ValueP PackageChoiceValueP;

// ----------------------------------------------------------------------------- : EOF
#endif
