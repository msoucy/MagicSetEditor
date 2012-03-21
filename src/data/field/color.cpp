//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field/color.hpp>
#include <script/script.hpp>

DECLARE_TYPEOF_COLLECTION(ColorField::ChoiceP);

// ----------------------------------------------------------------------------- : ColorField

ColorField::ColorField()
	: allow_custom(true)
{}

IMPLEMENT_FIELD_TYPE(Color, "color");

IMPLEMENT_REFLECTION(ColorField) {
	REFLECT_BASE(Field);
	REFLECT(allow_custom);
	REFLECT(choices);
}

// ----------------------------------------------------------------------------- : ColorField::Choice

IMPLEMENT_REFLECTION(ColorField::Choice) {
	REFLECT_IF_READING_COMPOUND_OR(true) {
		REFLECT(name);
		REFLECT(color);
	} else {
		REFLECT_NAMELESS(name);
		color = parse_color(name);
	}
}

// ----------------------------------------------------------------------------- : ColorStyle

ColorStyle::ColorStyle(const ColorFieldP& field)
	: Style(field)
	, radius(0)
	, left_width(100000), right_width (100000)
	, top_width (100000), bottom_width(100000)
	, combine(COMBINE_NORMAL)
{}

IMPLEMENT_REFLECTION(ColorStyle) {
	REFLECT_BASE(Style);
	REFLECT(radius);
	REFLECT(left_width);
	REFLECT(right_width);
	REFLECT(top_width);
	REFLECT(bottom_width);
	REFLECT(combine);
}

int ColorStyle::update(Context& ctx) {
	return Style::update(ctx);
}
