//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// -----------------------------------------------------------------------------
// : Includes

#include <util/prec.hpp>
#include <data/field/boolean.hpp>

// -----------------------------------------------------------------------------
// : BooleanField

BooleanField::BooleanField() {
    choices->choices.push_back(intrusive(new Choice((L"true"), (L"yes"))));
    choices->choices.push_back(intrusive(new Choice((L"false"), (L"no"))));
    choices->initIds();
    initial = script_true;
}

IMPLEMENT_FIELD_TYPE(Boolean, "boolean");

IMPLEMENT_REFLECTION(BooleanField) {
    REFLECT_BASE(Field); // NOTE: don't reflect as a ChoiceField, because we
                         // don't want to add extra choices
}

// -----------------------------------------------------------------------------
// : BooleanStyle

BooleanStyle::BooleanStyle(const ChoiceFieldP &field) : ChoiceStyle(field) {
    render_style = RENDER_BOTH;
    choice_images[(L"true")] =
        ScriptableImage(intrusive(new BuiltInImage((L"bool_yes"))));
    choice_images[(L"false")] =
        ScriptableImage(intrusive(new BuiltInImage((L"bool_no"))));
}

IMPLEMENT_REFLECTION(BooleanStyle) { REFLECT_BASE(ChoiceStyle); }

void BooleanStyle::after_reading(Version ver) {
    // Prior to 2.0.1, the choices were called "yes" and "no"
    if (!choice_images[(L"true")].isScripted() &&
        choice_images.find((L"yes")) != choice_images.end()) {
        choice_images[(L"true")] = choice_images[(L"yes")];
    }
    if (!choice_images[(L"false")].isScripted() &&
        choice_images.find((L"no")) != choice_images.end()) {
        choice_images[(L"false")] = choice_images[(L"no")];
    }
}
