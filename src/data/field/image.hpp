//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
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

/// A field for image values
class ImageField : public Field {
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

typedef Value ImageValue;
typedef ValueP ImageValueP;

// ----------------------------------------------------------------------------- : EOF
#endif
