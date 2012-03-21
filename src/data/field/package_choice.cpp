//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field/package_choice.hpp>
#include <util/io/package_manager.hpp>

// ----------------------------------------------------------------------------- : PackageChoiceField

IMPLEMENT_FIELD_TYPE(PackageChoice, "package choice");

IMPLEMENT_REFLECTION(PackageChoiceField) {
	REFLECT_BASE(Field);
	REFLECT(match);
	REFLECT(required);
	REFLECT(empty_name);
}

// ----------------------------------------------------------------------------- : PackageChoiceStyle

PackageChoiceStyle::PackageChoiceStyle(const PackageChoiceFieldP& field)
	: Style(field)
{}

int PackageChoiceStyle::update(Context& ctx) {
	return Style     ::update(ctx)
	     | font       .update(ctx) * CHANGE_OTHER;
}
/*void PackageChoiceStyle::initDependencies(Context& ctx, const Dependency& dep) const {
	Style     ::initDependencies(ctx, dep);
//	font       .initDependencies(ctx, dep);
}*/

IMPLEMENT_REFLECTION(PackageChoiceStyle) {
	REFLECT_BASE(Style);
	REFLECT(font);
}
