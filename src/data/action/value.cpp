//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
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

/// Swap the value in a Value object with a new one
#if !USE_SCRIPT_VALUE_CHOICE
inline void swap_value(ChoiceValue&         a, ChoiceValue        ::ValueType& b) { swap(a.value,    b); }
#endif
#if !USE_SCRIPT_VALUE_COLOR
inline void swap_value(ColorValue&          a, ColorValue         ::ValueType& b) { swap(a.value,    b); }
#endif
#if USE_SCRIPT_VALUE_VALUE
inline void swap_value(AnyValue&            a, AnyValue           ::ValueType& b) { swap(a.value,    b); }
#endif
#if !USE_SCRIPT_VALUE_IMAGE
inline void swap_value(ImageValue&          a, ImageValue         ::ValueType& b) { swap(a.filename, b); }
#endif
#if !USE_SCRIPT_VALUE_SYMBOL
inline void swap_value(SymbolValue&         a, SymbolValue        ::ValueType& b) { swap(a.filename, b); }
#endif
#if !USE_SCRIPT_VALUE_TEXT
inline void swap_value(TextValue&           a, TextValue          ::ValueType& b) { swap(a.value,    b); }
#endif
#if !USE_SCRIPT_VALUE_PACKAGE
inline void swap_value(PackageChoiceValue&  a, PackageChoiceValue ::ValueType& b) { swap(a.package_name, b); }
#endif

/// A ValueAction that swaps between old and new values
template <typename T, bool ALLOW_MERGE>
class SimpleValueAction : public ValueAction {
  public:
	inline SimpleValueAction(const intrusive_ptr<T>& value, const typename T::ValueType& new_value)
		: ValueAction(value), new_value(new_value)
	{}
	
	virtual void perform(bool to_undo) {
		ValueAction::perform(to_undo);
		swap_value(static_cast<T&>(*valueP), new_value);
		valueP->onAction(*this, to_undo); // notify value
	}
	
	virtual bool merge(const Action& action) {
		if (!ALLOW_MERGE) return false;
		TYPE_CASE(action, SimpleValueAction) {
			if (action.valueP == valueP) {
				// adjacent actions on the same value, discard the other one,
				// because it only keeps an intermediate value
				return true;
			}
		}
		return false;
	}
	
  private:
	typename T::ValueType new_value;
};

#if !USE_SCRIPT_VALUE_CHOICE
ValueAction* value_action(const ChoiceValueP&         value, const Defaultable<String>& new_value) { return new SimpleValueAction<ChoiceValue,         true> (value, new_value); }
#endif
#if !USE_SCRIPT_VALUE_COLOR
ValueAction* value_action(const ColorValueP&          value, const Defaultable<Color>&  new_value) { return new SimpleValueAction<ColorValue,          true> (value, new_value); }
#endif
#if USE_SCRIPT_VALUE_VALUE
ValueAction* value_action(const AnyValueP&            value, const ScriptValueP&        new_value) { return new SimpleValueAction<AnyValue,            false>(value, new_value); }
#endif
#if !USE_SCRIPT_VALUE_IMAGE
ValueAction* value_action(const ImageValueP&          value, const FileName&            new_value) { return new SimpleValueAction<ImageValue,          false>(value, new_value); }
#endif
#if !USE_SCRIPT_VALUE_SYMBOL
ValueAction* value_action(const SymbolValueP&         value, const FileName&            new_value) { return new SimpleValueAction<SymbolValue,         false>(value, new_value); }
#endif
#if !USE_SCRIPT_VALUE_PACKAGE
ValueAction* value_action(const PackageChoiceValueP&  value, const String&              new_value) { return new SimpleValueAction<PackageChoiceValue,  false>(value, new_value); }
#endif


// ----------------------------------------------------------------------------- : MultipleChoice

#if USE_SCRIPT_VALUE_CHOICE
ValueAction* value_action(const MultipleChoiceValueP& value, const ScriptValueP& new_value, const String& last_change) {
#else
ValueAction* value_action(const MultipleChoiceValueP& value, const Defaultable<String>& new_value, const String& last_change) {
#endif
	return new MultipleChoiceValueAction(value,new_value,last_change);
}

// copy paste of SimpleValueAction :(
// TODO: do this in a better way

void MultipleChoiceValueAction::perform(bool to_undo) {
	ValueAction::perform(to_undo);
	swap_value(static_cast<MultipleChoiceValue&>(*valueP), new_value);
	valueP->onAction(*this, to_undo); // notify value
}


// ----------------------------------------------------------------------------- : Text

#if USE_SCRIPT_VALUE_TEXT
TextValueAction::TextValueAction(const TextValueP& value, size_t start, size_t end, size_t new_end, const ScriptValueP& new_value, const String& name)
#else
TextValueAction::TextValueAction(const TextValueP& value, size_t start, size_t end, size_t new_end, const Defaultable<String>& new_value, const String& name)
#endif
	: ValueAction(value)
	, selection_start(start), selection_end(end), new_selection_end(new_end)
	, new_value(new_value)
	, name(name)
{}

String TextValueAction::getName(bool to_undo) const { return name; }

void TextValueAction::perform(bool to_undo) {
	ValueAction::perform(to_undo);
	swap_value(value(), new_value);
	swap(selection_end, new_selection_end);
	valueP->onAction(*this, to_undo); // notify value
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
