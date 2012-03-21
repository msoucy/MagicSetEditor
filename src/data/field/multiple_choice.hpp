//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FIELD_MULTIPLE_CHOICE
#define HEADER_DATA_FIELD_MULTIPLE_CHOICE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field/choice.hpp>

// ----------------------------------------------------------------------------- : MultipleChoiceField

DECLARE_POINTER_TYPE(MultipleChoiceField);
DECLARE_POINTER_TYPE(MultipleChoiceStyle);
DECLARE_POINTER_TYPE(MultipleChoiceValue);

/// A ChoiceField where multiple choices can be selected simultaniously
class MultipleChoiceField : public ChoiceField {
  public:
	MultipleChoiceField();
	DECLARE_FIELD_TYPE();
	
	UInt minimum_selection, maximum_selection; ///< How many choices can be selected simultaniously?
	String empty_choice; ///< Name to use when nothing is selected
};

// ----------------------------------------------------------------------------- : MultipleChoiceStyle

/// The Style for a MultipleChoiceField
class MultipleChoiceStyle : public ChoiceStyle {
  public:
	MultipleChoiceStyle(const MultipleChoiceFieldP& field);
	DECLARE_STYLE_TYPE(MultipleChoice);
	
	Scriptable<Direction> direction;	///< In what direction are choices layed out?
	Scriptable<double> spacing;			///< Spacing between choices (images) in pixels
	
	virtual int  update(Context&);
};

// ----------------------------------------------------------------------------- : MultipleChoiceValue

/// The Value in a MultipleChoiceField
/** The value is stored as "<choice>, <choice>, <choice>"
 *  The choices must be ordered by id
 */
class MultipleChoiceValue : public ChoiceValue {
  public:
	inline MultipleChoiceValue(const MultipleChoiceFieldP& field) : ChoiceValue(field) {}
	DECLARE_HAS_FIELD(MultipleChoice);
	virtual ValueP clone() const;
	
	/// Splits the value, stores the selected choices in the out parameter
	void get(vector<String>& out) const;
	
	virtual bool update(Context&, const Action* = nullptr);
	
  private:
	DECLARE_REFLECTION();
	
	/// Put the value in normal form (all choices ordered, empty_name
	void normalForm();
};

// ----------------------------------------------------------------------------- : Utilities

/// Is the given choice selected in the value?
bool chosen(const String& multiple_choice_value, const String& chioce);

// ----------------------------------------------------------------------------- : EOF
#endif
