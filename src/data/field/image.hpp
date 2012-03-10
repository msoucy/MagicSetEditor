//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FIELD_IMAGE
#define HEADER_DATA_FIELD_IMAGE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/field.hpp>
#include <script/scriptable.hpp>
#include <script/image.hpp>

// ----------------------------------------------------------------------------- : ImageField

DECLARE_POINTER_TYPE(ImageField);
DECLARE_POINTER_TYPE(ImageStyle);
#if !USE_SCRIPT_VALUE_IMAGE
DECLARE_POINTER_TYPE(ImageValue);
#endif

/// A field for image values
#if USE_SCRIPT_VALUE_IMAGE
class ImageField : public AnyField {
#else
class ImageField : public Field {
#endif
  public:
	// no extra data
	DECLARE_FIELD_TYPE();
};

// ----------------------------------------------------------------------------- : ImageStyle

/// The Style for a ImageField
class ImageStyle : public Style {
  public:
	inline ImageStyle(const ImageFieldP& field) : Style(field) {}
	DECLARE_STYLE_TYPE(Image);
	
	ScriptableImage default_image; ///< Placeholder
	
	virtual int update(Context&);
};

// ----------------------------------------------------------------------------- : ImageValue

#if USE_SCRIPT_VALUE_IMAGE
typedef AnyValue ImageValue;
typedef AnyValueP ImageValueP;
#else
/// The Value in a ImageField, i.e. an image
class ImageValue : public Value {
  public:
	inline ImageValue(const ImageFieldP& field) : Value(field) {}
	DECLARE_VALUE_TYPE(Image, FileName);
	
	ValueType filename;    ///< Filename of the image (in the current package), or ""
};
#endif

// ----------------------------------------------------------------------------- : EOF
#endif
