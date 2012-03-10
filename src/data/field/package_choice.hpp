//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
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
#if !USE_SCRIPT_VALUE_PACKAGE
DECLARE_POINTER_TYPE(PackageChoiceValue);
#endif

/// A field for PackageChoice values, it contains a list of choices for PackageChoices
#if USE_SCRIPT_VALUE_PACKAGE
class PackageChoiceField : public AnyField {
#else
class PackageChoiceField : public Field {
#endif
  public:
	PackageChoiceField() : required(true), empty_name(_("none")) {}
	DECLARE_FIELD_TYPE();
	
	String             match;			///< Glob of package filenames to match
	bool               required;		///< Is selecting a package required?
	String             empty_name;		///< Displayed name for the empty value (if !required)
#if !USE_SCRIPT_VALUE_PACKAGE
	OptionalScript     script;			///< Script to apply to all values
	String             initial;			///< Initial value
	
	virtual void initDependencies(Context&, const Dependency&) const;
#endif
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

#if USE_SCRIPT_VALUE_PACKAGE
typedef AnyValue PackageChoiceValue;
typedef AnyValueP PackageChoiceValueP;
#else
/// The Value in a PackageChoiceField
class PackageChoiceValue : public Value {
  public:
	PackageChoiceValue(const PackageChoiceFieldP& field) : Value(field), package_name(field->initial) {}
	DECLARE_VALUE_TYPE(PackageChoice, String);
	
	ValueType package_name;	///< The selected package
	
	/// Get the package (if it is set)
	PackagedP getPackage() const;
	
	virtual bool update(Context&, const Action* = nullptr);
};
#endif

// ----------------------------------------------------------------------------- : EOF
#endif
