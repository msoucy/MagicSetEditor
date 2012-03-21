//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/action/value.hpp>
#include <data/field.hpp>
#include <data/field/text.hpp>
#include <data/field/choice.hpp>
#include <data/field/multiple_choice.hpp>
#include <data/field/color.hpp>
#include <data/field/image.hpp>
#include <data/field/symbol.hpp>
#include <data/field/package_choice.hpp>
#include <data/card.hpp>
#include <util/tagged_string.hpp>
#include <data/set.hpp> // for ValueActionPerformer

// ----------------------------------------------------------------------------- : ValueAction

String ValueAction::getName(bool to_undo) const {
	return _ACTION_1_("change", valueP->fieldP->name);
}

void ValueAction::perform(bool to_undo) {
	if (card) {
		swap(const_cast<Card*>(card)->time_modified, old_time_modified);
	}
}

void ValueAction::isOnCard(Card* card) {
	this->card = card;
}

// ----------------------------------------------------------------------------- : Simple

void SimpleValueAction::perform(bool to_undo) {
	ValueAction::perform(to_undo);
	swap(valueP->value, new_value);
	valueP->onAction(*this, to_undo); // notify value
}

bool SimpleValueAction::merge(const Action& action) {
	if (!allow_merge) return false;
	TYPE_CASE(action, SimpleValueAction) {
		if (action.valueP == valueP) {
			// adjacent actions on the same value, discard the other one,
			// because it only keeps an intermediate value
			return true;
		}
	}
	return false;
}

ValueAction* value_action(const ValueP& value, const ScriptValueP& new_value) {
	return new SimpleValueAction(value, new_value);
}

ValueAction* value_action(const MultipleChoiceValueP& value, const ScriptValueP& new_value, const String& last_change) {
	return new MultipleChoiceValueAction(value,new_value,last_change);
}

// ----------------------------------------------------------------------------- : Text

TextValueAction::TextValueAction(const TextValueP& value, size_t start, size_t end, size_t new_end, const ScriptValueP& new_value, const String& name)
	: SimpleValueAction(value, new_value)
	, selection_start(start), selection_end(end), new_selection_end(new_end)
	, name(name)
{}

String TextValueAction::getName(bool to_undo) const { return name; }

void TextValueAction::perform(bool to_undo) {
	swap(selection_end, new_selection_end);
	SimpleValueAction::perform(to_undo);
}

bool TextValueAction::merge(const Action& action) {
	TYPE_CASE(action, TextValueAction) {
		if (&action.value() == &value() && action.name == name) {
			if (action.selection_start == selection_end) {
				// adjacent edits, keep old value of this, it is older
				selection_end = action.selection_end;
				return true;
			} else if (action.new_selection_end == selection_start && name == _ACTION_("backspace")) {
				// adjacent backspaces
				selection_start = action.selection_start;
				selection_end   = action.selection_end;
				return true;
			}
		}
	}
	return false;
}

TextValue& TextValueAction::value() const {
	return static_cast<TextValue&>(*valueP);
}


TextValueAction* toggle_format_action(const TextValueP& value, const String& tag, size_t start_i, size_t end_i, size_t start, size_t end, const String& action_name) {
	if (start > end) {
		swap(start, end);
		swap(start_i, end_i);
	}
	String new_value;
	String old_value = value->value->toString();
	// Are we inside the tag we are toggling?
	if (!is_in_tag(old_value, _("<") + tag, start_i, end_i)) {
		// we are not inside this tag, add it
		new_value =  old_value.substr(0, start_i);
		new_value += _("<") + tag + _(">");
		new_value += old_value.substr(start_i, end_i - start_i);
		new_value += _("</") + tag + _(">");
		new_value += old_value.substr(end_i);
	} else {
		// we are inside this tag, 'remove' it
		new_value =  old_value.substr(0, start_i);
		new_value += _("</") + tag + _(">");
		new_value += old_value.substr(start_i, end_i - start_i);
		new_value += _("<") + tag + _(">");
		new_value += old_value.substr(end_i);
	}
	// Build action
	if (start != end) {
		// don't simplify if start == end, this way we insert <b></b>, allowing the
		// user to press Ctrl+B and start typing bold text
		new_value = simplify_tagged(new_value);
	}
	if (new_value == old_value) {
		return nullptr; // no changes
	} else {
		return new TextValueAction(value, start, end, end, to_script(new_value), action_name);
	}
}

TextValueAction* typing_action(const TextValueP& value, size_t start_i, size_t end_i, size_t start, size_t end, const String& replacement, const String& action_name)  {
	bool reverse = start > end;
	if (reverse) {
		swap(start, end);
		swap(start_i, end_i);
	}
	String old_value = value->value->toString();
	String new_value = tagged_substr_replace(old_value, start_i, end_i, replacement);
	if (new_value == old_value) {
		// no change
		return nullptr;
	} else {
		if (reverse) {
			return new TextValueAction(value, end, start, start+untag(replacement).size(), to_script(new_value), action_name);
		} else {
			return new TextValueAction(value, start, end, start+untag(replacement).size(), to_script(new_value), action_name);
		}
	}
}

// ----------------------------------------------------------------------------- : Reminder text

TextToggleReminderAction::TextToggleReminderAction(const TextValueP& value, size_t pos_in)
	: ValueAction(value)
{
	String old_value = value->value->toString();
	pos = in_tag(old_value, _("<kw-"), pos_in, pos_in);
	if (pos == String::npos) {
		throw InternalError(_("TextToggleReminderAction: not in <kw- tag"));
	}
	Char c = old_value.GetChar(pos + 4);
	enable = !(c == _('1') || c == _('A')); // if it was not enabled, then enable it
	old = enable ? _('1') : _('0');
}
String TextToggleReminderAction::getName(bool to_undo) const {
	return enable ? _("Show reminder text") : _("Hide reminder text");
}

void TextToggleReminderAction::perform(bool to_undo) {
	ValueAction::perform(to_undo);
	TextValue& value = static_cast<TextValue&>(*valueP);
	String val = value.value->toString();
	assert(pos + 4 < val.size());
	size_t end = match_close_tag(val, pos);
	Char& c = val[pos + 4];
	swap(c, old);
	if (end != String::npos && end + 5 < val.size()) {
		val[end + 5] = c; // </kw-c>
	}
	value.value = to_script(val);
	value.onAction(*this, to_undo); // notify value
}


// ----------------------------------------------------------------------------- : Event

String ScriptValueEvent::getName(bool) const {
	assert(false); // this action is just an event, getName shouldn't be called
	throw InternalError(_("ScriptValueEvent::getName"));
}
void ScriptValueEvent::perform(bool) {
	assert(false); // this action is just an event, it should not be performed
}


String ScriptStyleEvent::getName(bool) const {
	assert(false); // this action is just an event, getName shouldn't be called
	throw InternalError(_("ScriptStyleEvent::getName"));
}
void ScriptStyleEvent::perform(bool) {
	assert(false); // this action is just an event, it should not be performed
}

// ----------------------------------------------------------------------------- : Action performer

ValueActionPerformer::ValueActionPerformer(const ValueP& value, Card* card, const SetP& set)
	: value(value), card(card), set(set)
{}
ValueActionPerformer::~ValueActionPerformer() {}

void ValueActionPerformer::addAction(ValueAction* action) {
	action->isOnCard(card);
	set->actions.addAction(action);
}

Package& ValueActionPerformer::getLocalPackage() {
	return *set;
}
