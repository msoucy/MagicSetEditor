// //+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field.hpp>
#include <data/field/text.hpp>
#include <data/field/choice.hpp>
#include <data/field/multiple_choice.hpp>
#include <data/field/boolean.hpp>
#include <data/field/image.hpp>
#include <data/field/symbol.hpp>
#include <data/field/color.hpp>
#include <data/field/information.hpp>
#include <data/field/package_choice.hpp>
#include <util/error.hpp>

DECLARE_TYPEOF_COLLECTION(StyleListener*);

DECLARE_DYNAMIC_ARG(Field const*, field_for_reading);
IMPLEMENT_DYNAMIC_ARG(Field const*, field_for_reading, nullptr);

// ----------------------------------------------------------------------------- : Field

Field::Field()
	: index            (0) // sensible default?
	, editable         (true)
	, save_value       (true)
	, show_statistics  (true)
	, position_hint    (0)
	, identifying      (false)
	, card_list_column (100)
	, card_list_width  (100)
	, card_list_visible(false)
	, card_list_allow  (true)
	, card_list_align  (ALIGN_LEFT)
	, default_name     (_("Default"))
	, initial          (script_default_nil)
{}

Field::~Field() {}

void Field::initDependencies(Context& ctx, const Dependency& dep) const {
	script        .initDependencies(ctx, dep);
	default_script.initDependencies(ctx, dep);
	sort_script   .initDependencies(ctx, dep);
}

IMPLEMENT_REFLECTION(Field) {
	REFLECT_IF_NOT_READING {
		String type = typeName();
		REFLECT(type);
	}
	REFLECT(name);
	REFLECT(caption);
	REFLECT(description);
	REFLECT_N("icon", icon_filename);
	REFLECT(editable);
	REFLECT(save_value);
	REFLECT(show_statistics);
	REFLECT(position_hint);
	REFLECT(identifying);
	REFLECT(card_list_column);
	REFLECT(card_list_width);
	REFLECT(card_list_visible);
	REFLECT(card_list_allow);
	REFLECT(card_list_name);
	REFLECT_N("card_list_alignment", card_list_align);
	REFLECT(script);
	REFLECT_N("default", default_script);
	REFLECT(sort_script);
	REFLECT(default_name);
	WITH_DYNAMIC_ARG(field_for_reading, this);
	REFLECT(initial);
}

void Field::after_reading(Version ver) {
	name = canonical_name_form(name);
	if(caption.empty()) caption = name_to_caption(name);
	if(card_list_name.empty()) card_list_name = capitalize(caption);
}

template <>
intrusive_ptr<Field> read_new<Field>(Reader& reader) {
	// there must be a type specified
	String type;
	reader.handle(_("type"), type);
	if      (type == _("text"))				return intrusive(new TextField());
	else if (type == _("choice"))			return intrusive(new ChoiceField());
	else if (type == _("multiple choice"))	return intrusive(new MultipleChoiceField());
	else if (type == _("boolean"))			return intrusive(new BooleanField());
	else if (type == _("image"))			return intrusive(new ImageField());
	else if (type == _("symbol"))			return intrusive(new SymbolField());
	else if (type == _("color"))			return intrusive(new ColorField());
	else if (type == _("info"))				return intrusive(new InfoField());
	else if (type == _("package choice"))	return intrusive(new PackageChoiceField());
	else if (type.empty()) {
		reader.warning(_ERROR_1_("expected key", _("type")));
		throw ParseError(_ERROR_("aborting parsing"));
	} else {
		reader.warning(_ERROR_1_("unsupported field type", type));
		throw ParseError(_ERROR_("aborting parsing"));
	}
}

// ----------------------------------------------------------------------------- : Style

Style::Style(const FieldP& field)
	: fieldP(field)
	, z_index(0)
	, tab_index(0)
	, left (1000000), top   (1000000)
	, width(0),       height(0)
	, right(1000000), bottom(1000000)
	, angle(0)
	, visible(true)
	, automatic_side(AUTO_UNKNOWN)
	, content_dependent(false)
{}

Style::~Style() {}

IMPLEMENT_REFLECTION(Style) {
	REFLECT(z_index);
	REFLECT(tab_index);
	REFLECT(left);
	REFLECT(width);
	REFLECT(right);
	REFLECT(top);
	REFLECT(height);
	REFLECT(bottom);
	REFLECT(angle);
	REFLECT(visible);
	REFLECT(mask);
}

void init_object(const FieldP& field, StyleP& style) {
	if (!style) style = field->newStyle();
}
template <> StyleP read_new<Style>(Reader&) {
	throw InternalError(_("IndexMap contains nullptr StyleP the application should have crashed already"));
}

inline bool is_set(const Scriptable<double>& x) {
	return x.isScripted() || x < 100000;
}
inline bool is_setw(const Scriptable<double>& x) {
	return x.isScripted() || fabs(x()) > 0.001;
}

int Style::update(Context& ctx) {
	int changed =
	     ( left   .update(ctx)
	     | width  .update(ctx)
	     | right  .update(ctx)
	     | top    .update(ctx)
	     | height .update(ctx)
	     | bottom .update(ctx)
	     | angle  .update(ctx) ) * CHANGE_SIZE
	     | visible.update(ctx)   * CHANGE_OTHER
	     | mask   .update(ctx)   * CHANGE_MASK;
	// determine automatic_side and attachment of rotation point
	if (automatic_side == AUTO_UNKNOWN) {
		if      (!is_set (right))  automatic_side = (AutomaticSide)(automatic_side | AUTO_RIGHT);
		else if (!is_setw(width))  automatic_side = (AutomaticSide)(automatic_side | AUTO_WIDTH);
		else if (!is_set (left))   automatic_side = (AutomaticSide)(automatic_side | AUTO_LEFT);
		else                       automatic_side = (AutomaticSide)(automatic_side | AUTO_LR);
		if      (!is_set (bottom)) automatic_side = (AutomaticSide)(automatic_side | AUTO_BOTTOM);
		else if (!is_setw(height)) automatic_side = (AutomaticSide)(automatic_side | AUTO_HEIGHT);
		else if (!is_set (top))    automatic_side = (AutomaticSide)(automatic_side | AUTO_TOP);
		else                       automatic_side = (AutomaticSide)(automatic_side | AUTO_TB);
		changed |= CHANGE_SIZE;
	}
	if (!changed) return CHANGE_NONE;
	// update the automatic_side
	if      (automatic_side & AUTO_LEFT)   left   = right - width;
	else if (automatic_side & AUTO_WIDTH)  width  = right - left;
	else if (automatic_side & AUTO_RIGHT)  right  = left + width;
	else                                   {int lr = int(left + right); left = (lr - width) / 2; right = (lr + width) / 2; }
	if      (automatic_side & AUTO_TOP)    top    = bottom - height;
	else if (automatic_side & AUTO_HEIGHT) height = bottom - top;
	else if (automatic_side & AUTO_BOTTOM) bottom = top + height;
	else                                   {int tb = int(top + bottom); top = (tb - height) / 2; bottom = (tb + height) / 2; }
	// adjust rotation point
	if (angle != 0 && (automatic_side & (AUTO_LEFT | AUTO_TOP))) {
		double s = sin(deg_to_rad(angle)), c = cos(deg_to_rad(angle));
		if (automatic_side & AUTO_LEFT) { // attach right corner instead of left
			left = left + width * (1 - c);
			top  = top  + width * s;
		}
		if (automatic_side & AUTO_TOP) { // attach botom corner instead of top
			left = left - height * s;
			top  = top  + height * (1 - c);
		}
	}
	if (width  < 0) width  = -width;
	if (height < 0) height = -height;
	// done
	return changed;
}

bool Style::isVisible() const {
	return visible
	    &&     (width())  > 0      
	    && fabs(left())   < 100000
	    && fabs(right())  < 100000
	    &&     (height()) > 0      
	    && fabs(top())    < 100000
	    && fabs(bottom()) < 100000;
}
bool Style::hasSize() const {
	int h = is_setw(width)  + is_set(left) + is_set(right);
	int v = is_setw(height) + is_set(top)  + is_set(bottom);
	return h >= 2 && v >= 2;
}

void Style::initDependencies(Context& ctx, const Dependency& dep) const {
//	left   .initDependencies(ctx,dep);
//	top    .initDependencies(ctx,dep);
//	width  .initDependencies(ctx,dep);
//	height .initDependencies(ctx,dep);
//	visible.initDependencies(ctx,dep);
}
void Style::checkContentDependencies(Context& ctx, const Dependency& dep) const {
	left   .initDependencies(ctx,dep);
	top    .initDependencies(ctx,dep);
	width  .initDependencies(ctx,dep);

	height .initDependencies(ctx,dep);
	right  .initDependencies(ctx,dep);
	bottom .initDependencies(ctx,dep);
	visible.initDependencies(ctx,dep);
	mask   .initDependencies(ctx,dep);
}

void Style::markDependencyMember(const String& name, const Dependency& dep) const {
	// mark dependencies on content
	if (dep.type == DEP_DUMMY && dep.index == false && starts_with(name, _("content "))) {
		// anything that starts with "content_" is a content property
		const_cast<Dependency&>(dep).index = true;
	}
}

void mark_dependency_member(const Style& style, const String& name, const Dependency& dep) {
	style.markDependencyMember(name,dep);
}

// ----------------------------------------------------------------------------- : StyleListener

void Style::addListener(StyleListener* listener) {
	listeners.push_back(listener);
}
void Style::removeListener(StyleListener* listener) {
	listeners.erase(
		std::remove(
			listeners.begin(),
			listeners.end(),
			listener
			),
		listeners.end()
		);
}
void Style::tellListeners(int changes) {
	FOR_EACH(l, listeners) l->onStyleChange(changes);
}

StyleListener::StyleListener(const StyleP& style)
	: styleP(style)
{
	style->addListener(this);
}
StyleListener::~StyleListener() {
	styleP->removeListener(this);
}


// ----------------------------------------------------------------------------- : Value : reflecting ScriptValues
// TODO: move this to a more sensible location

	// possible values:
	//  * "quoted string"           # a string
	//  * 123.456                   # a number
	//  * fileref("quoted-string")  # reference to a file in the set
	//  * rgb(123,123,123)          # a color
	//  * nil                       # nil
	//  * true/false                # a boolean
	//  * mark_default(value)       # a value marked as being default

void parse_errors_to_reader_warnings(Reader& reader, vector<ScriptParseError> const& errors);
ScriptValueP script_local_image_file(LocalFileName const& filename);
ScriptValueP script_local_symbol_file(LocalFileName const& filename);

void Reader::handle(ScriptValueP& value) {
	Field const* field = field_for_reading();
	if (formatVersion() < 20001 && field) {
		// in older versions, the format was based on the type of the field
		if (dynamic_cast<BooleanField const*>(field)) {
			// boolean field: boolean "yes" or "no"
			bool x;
			handle(x);
			value = to_script(x);
		} else if (dynamic_cast<TextField const*>(field) || dynamic_cast<ChoiceField const*>(field) || dynamic_cast<PackageChoiceField const*>(field)) {
			// text, choice fields: string
			String str;
			handle(str);
			value = to_script(str);
		} else if (dynamic_cast<ColorField const*>(field)) {
			// color field: color
			Color x;
			handle(x);
			value = to_script(x);
		} else if (dynamic_cast<ImageField const*>(field)) {
			// image, symbol fields: string that is a filename in the set
			String str;
			handle(str);
			value = script_local_image_file(LocalFileName::fromReadString(str));
		} else if (dynamic_cast<SymbolField const*>(field)) {
			// image, symbol fields: string that is a filename in the set
			String str;
			handle(str);
			value = script_local_symbol_file(LocalFileName::fromReadString(str));
		} else if (dynamic_cast<InfoField const*>(field)) {
			// this should never happen, since info fields were not saved
			String str;
			handle(str);
		} else {
			throw InternalError(_("Reader::handle(ScriptValueP)"));
		}
	} else {
		// in the new system, the type is stored in the file.
		String unparsed;
		handle(unparsed);
		if (unparsed.empty()) {
			value = script_default_nil;
		} else {
			vector<ScriptParseError> errors;
			value = parse_value(unparsed, this->getPackage(), errors);
			if (!value) {
				value = script_default_nil;
			}
			parse_errors_to_reader_warnings(*this, errors);
		}
	}
}
void Writer::handle(ScriptValueP const& value) {
	// TODO: Make a distinction in which values can be saved?
	handle(value->toCode());
}

// ----------------------------------------------------------------------------- : Value

IMPLEMENT_DYNAMIC_ARG(Value*, value_being_updated, nullptr);

Value::Value(const FieldP& field)
	: fieldP(field), value(field->initial)
{
	assert(value);
}

Value::Value(const FieldP& field, const ScriptValueP& value)
	: fieldP(field), value(value)
{
	assert(value);
}

Value::~Value() {}

ValueP Value::clone() const {
	return intrusive(new Value(*this));
}

bool Value::equals(const Value* that) {
	return this == that;
}

String Value::toFriendlyString() const {
	try {
		return value->toFriendlyString();
	} catch (...) {
		return _("<") + value->typeName() + _(">");
	}
}

bool Value::update(Context& ctx, const Action* act) {
	WITH_DYNAMIC_ARG(last_update_age,     last_modified.get()); // TODO: this is redundant, since it can be got from value_being_updated
	WITH_DYNAMIC_ARG(value_being_updated, this);
	bool change = false;
	if (ScriptDefault const* dv = dynamic_cast<ScriptDefault*>(value.get())) {
		ScriptValueP dvv = dv->un_default;
		change = fieldP->default_script.invokeOn(ctx, dvv);
		change |= fieldP->script.invokeOn(ctx, dvv);
		if (change) value = make_default(dvv);
	} else {
		change = fieldP->script.invokeOn(ctx, value);
	}
	updateSortValue(ctx);
	return change;
}

void Value::updateSortValue(Context& ctx) {
	sort_value = fieldP->sort_script.invoke(ctx)->toString();
}


IMPLEMENT_REFLECTION_NAMELESS(Value) {
	if (reflector.isWriting() && !fieldP->save_value) return;
	if (reflector.isWriting() && is_default(value)) return;
	WITH_DYNAMIC_ARG(field_for_reading, fieldP.get());
	REFLECT_NAMELESS(value);
}

void init_object(const FieldP& field, ValueP& value) {
	if (!value) {
		value = field->newValue();
	}
}

template <> ValueP read_new<Value>(Reader&) {
	throw InternalError(_("IndexMap contains nullptr ValueP the application should have crashed already"));
}

void mark_dependency_member(const IndexMap<FieldP,ValueP>& value, const String& name, const Dependency& dep) {
	IndexMap<FieldP,ValueP>::const_iterator it = value.find(name);
	if (it != value.end()) {
		(*it)->fieldP->dependent_scripts.add(dep);
	}
}


