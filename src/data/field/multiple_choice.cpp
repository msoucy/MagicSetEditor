//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field/multiple_choice.hpp>
#include <data/action/value.hpp>

// ----------------------------------------------------------------------------- : MultipleChoiceField

MultipleChoiceField::MultipleChoiceField()
	: minimum_selection(0)
	, maximum_selection(1000000)
{}

IMPLEMENT_FIELD_TYPE(MultipleChoice, "multiple choice");

IMPLEMENT_REFLECTION(MultipleChoiceField) {
	REFLECT_BASE(ChoiceField);
	REFLECT(minimum_selection);
	REFLECT(maximum_selection);
	REFLECT(empty_choice);
}

// ----------------------------------------------------------------------------- : MultipleChoiceStyle

MultipleChoiceStyle::MultipleChoiceStyle(const MultipleChoiceFieldP& field)
	: ChoiceStyle(field)
	, direction(LEFT_TO_RIGHT)
	, spacing(0)
{}

IMPLEMENT_REFLECTION(MultipleChoiceStyle) {
	REFLECT_BASE(ChoiceStyle);
	REFLECT(direction);
	REFLECT(spacing);
}

int MultipleChoiceStyle::update(Context& ctx) {
	return ChoiceStyle::update(ctx)
	     | direction.update(ctx) * CHANGE_OTHER
	     | spacing.update(ctx) * CHANGE_OTHER;
}

// ----------------------------------------------------------------------------- : MultipleChoiceValue

IMPLEMENT_VALUE_CLONE(MultipleChoice);

IMPLEMENT_REFLECTION_NAMELESS(MultipleChoiceValue) {
	REFLECT_BASE(ChoiceValue);
}

bool MultipleChoiceValue::update(Context& ctx, const Action* act) {
	String old_value = value->toString();
	if (const MultipleChoiceValueAction* mvca = dynamic_cast<const MultipleChoiceValueAction*>(act)) {
		ctx.setVariable(_("last_change"), to_script(mvca->changed_choice));
	}
	ChoiceValue::update(ctx,act);
	normalForm();
	return value->toString() != old_value;
}

void MultipleChoiceValue::get(vector<String>& out) const {
	// split the value
	out.clear();
	bool is_new = true;
	String val = value->toString();
	FOR_EACH_CONST(c, val) {
		if (c == _(',')) {
			is_new = true;
		} else if (is_new) {
			if (c != _(' ')) { // ignore whitespace after ,
				is_new = false;
				out.push_back(String(1, c));
			}
		} else {
			assert(!out.empty());
			out.back() += c;
		}
	}
}

void MultipleChoiceValue::normalForm() {
	String val = value->toString();
	// which choices are active?
	vector<bool> seen(field().choices->lastId());
	for (size_t pos = 0 ; pos < val.size() ; ) {
		if (val.GetChar(pos) == _(' ')) {
			++pos; // ingore whitespace
		} else {
			// does this choice match the one asked about?
			size_t end = val.find_first_of(_(','), pos);
			if (end == String::npos) end = val.size();
			// find this choice
			for (size_t i = 0 ; i < seen.size() ; ++i) {
				if (is_substr(val, pos, field().choices->choiceName((int)i))) {
					seen[i] = true;
					break;
				}
			}
			pos = end + 1;
		}
	}
	// now put them back in the right order
	val.clear();
	for (size_t i = 0 ; i < seen.size() ; ++i) {
		if (seen[i]) {
			if (!val.empty()) val += _(", ");
			val += field().choices->choiceName((int)i);
		}
	}
	// empty choice name
	if (val.empty()) {
		val = field().empty_choice;
	}
	// store
	value = with_defaultness_of(value, to_script(val));
}
