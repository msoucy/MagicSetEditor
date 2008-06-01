//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/locale.hpp>
#include <data/game.hpp>
#include <data/stylesheet.hpp>
#include <data/symbol_font.hpp>
#include <util/io/package_manager.hpp>
#include <script/to_value.hpp>
#include <wx/wfstream.h>

#include <wx/stdpaths.h>
#if defined(__WXMSW__)
	#include <wx/mstream.h>
#endif

// ----------------------------------------------------------------------------- : Locale class

LocaleP the_locale;

String Locale::typeName() const { return _("locale"); }

LocaleP Locale::byName(const String& name) {
	return package_manager.open<Locale>(name + _(".mse-locale"));
}

IMPLEMENT_REFLECTION_NO_SCRIPT(Locale) {
	REFLECT_BASE(Packaged);
	REFLECT_N("menu",        translations[LOCALE_CAT_MENU]);
	REFLECT_N("help",        translations[LOCALE_CAT_HELP]);
	REFLECT_N("tool",        translations[LOCALE_CAT_TOOL]);
	REFLECT_N("tooltip",     translations[LOCALE_CAT_TOOLTIP]);
	REFLECT_N("label",       translations[LOCALE_CAT_LABEL]);
	REFLECT_N("button",      translations[LOCALE_CAT_BUTTON]);
	REFLECT_N("title",       translations[LOCALE_CAT_TITLE]);
	REFLECT_N("action",      translations[LOCALE_CAT_ACTION]);
	REFLECT_N("error",       translations[LOCALE_CAT_ERROR]);
	REFLECT_N("type",        translations[LOCALE_CAT_TYPE]);
	REFLECT_N("game",        game_translations);
	REFLECT_N("stylesheet",  stylesheet_translations);
	REFLECT_N("symbol font", symbol_font_translations);
}

IMPLEMENT_REFLECTION_NO_GET_MEMBER(SubLocale) {
	REFLECT_NAMELESS(translations);
}

// ----------------------------------------------------------------------------- : Translation

String warn_and_identity(const String& key) {
	handle_warning(_("Missing key in locale: ") + key, false);
	return key;
}

String SubLocale::tr(const String& key, DefaultLocaleFun def) {
	map<String,String>::const_iterator it = translations.find(key);
	if (it == translations.end()) {
		return def(key);
	} else {
		return it->second;
	}
}
String SubLocale::tr(const String& subcat, const String& key, DefaultLocaleFun def) {
	map<String,String>::const_iterator it = translations.find(subcat + _(" ") + key);
	if (it == translations.end()) {
		return def(key);
	} else {
		return it->second;
	}
}

// from util/locale.hpp

String tr(LocaleCategory cat, const String& key, DefaultLocaleFun def) {
	if (!the_locale) return def(key); // no locale loaded (yet)
	return the_locale->translations[cat].tr(key,def);
}

#define IMPLEMENT_TR_TYPE(Type, type_translations)												\
	String tr(const Type& t, const String& key, DefaultLocaleFun def) {							\
		if (!the_locale) return def(key);														\
		SubLocaleP loc = the_locale->type_translations[t.name()];								\
		if (!loc)        return def(key);														\
		return loc->tr(key, def);																\
	}																							\
	String tr(const Type& t, const String& subcat, const String& key, DefaultLocaleFun def) {	\
		if (!the_locale) return def(key);														\
		SubLocaleP loc = the_locale->type_translations[t.name()];								\
		if (!loc)        return def(key);														\
		return loc->tr(subcat, key, def);														\
	}

IMPLEMENT_TR_TYPE(Game,       game_translations)
IMPLEMENT_TR_TYPE(StyleSheet, stylesheet_translations)
IMPLEMENT_TR_TYPE(SymbolFont, symbol_font_translations)

// ----------------------------------------------------------------------------- : Validation

DECLARE_POINTER_TYPE(SubLocaleValidator);

class KeyValidator {
  public:
	int  args;
	bool optional;
	DECLARE_REFLECTION();
};

class SubLocaleValidator : public IntrusivePtrBase<SubLocaleValidator> {
  public:
	map<String,KeyValidator> keys; ///< Arg count for each key
	DECLARE_REFLECTION();
};

/// Validation information for locales
class LocaleValidator {
  public:
	map<String, SubLocaleValidatorP> sublocales;
	DECLARE_REFLECTION();
};

template <> void Reader::handle(KeyValidator& k) {
	String v = getValue();
	if (starts_with(v, _("optional, "))) {
		k.optional = true;
		v = v.substr(10);
	} else {
		k.optional = false;
	}
	long l = 0;
	v.ToLong(&l);
	k.args = l;
}
template <> void Writer::handle(const KeyValidator& v) {
	assert(false);
}
IMPLEMENT_REFLECTION_NO_SCRIPT(SubLocaleValidator) {
	REFLECT_NAMELESS(keys);
}
IMPLEMENT_REFLECTION_NO_SCRIPT(LocaleValidator) {
	REFLECT_NAMELESS(sublocales);
}

/// Count "%s" in str
int string_format_args(const String& str) {
	int count = 0;
	bool in_percent = false;
	FOR_EACH_CONST(c, str) {
		if (in_percent) {
			if (c == _('s')) {
				count++;
			}
			in_percent = false;
		} else if (c == _('%')) {
			in_percent = true;
		}
	}
	return count;
}

/// Load a text file from a resource
/** TODO: Move me
 */
InputStreamP load_resource_text(const String& name);
InputStreamP load_resource_text(const String& name) {
	#if defined(__WXMSW__)
		HRSRC hResource = ::FindResource(wxGetInstance(), name, _("TEXT"));
		if ( hResource == 0 ) throw InternalError(String::Format(_("Resource not found: %s"), name));
		HGLOBAL hData = ::LoadResource(wxGetInstance(), hResource);
		if ( hData == 0 ) throw InternalError(String::Format(_("Resource not text: %s"), name));
		char* data = (char *)::LockResource(hData);
		if ( !data ) throw InternalError(String::Format(_("Resource cannot be locked: %s"), name));
		int len = ::SizeofResource(wxGetInstance(), hResource);
		return new_shared2<wxMemoryInputStream>(data, len);
	#else
		static String path = wxStandardPaths::Get().GetDataDir() + _("/resource/") + name;
		return new_shared1<wxFileInputStream>(path);
	#endif
}


DECLARE_TYPEOF(map<String COMMA String>);
DECLARE_TYPEOF(map<String COMMA KeyValidator>);

void Locale::validate(Version ver) {
	Packaged::validate(ver);
	// load locale validator
	LocaleValidator v;
	Reader r(load_resource_text(_("expected_locale_keys")), nullptr, _("expected_locale_keys"));
	r.handle_greedy(v);
	// validate
	String errors;
	errors += translations[LOCALE_CAT_MENU   ].validate(_("menu"),    v.sublocales[_("menu")   ]);
	errors += translations[LOCALE_CAT_HELP   ].validate(_("help"),    v.sublocales[_("help")   ]);
	errors += translations[LOCALE_CAT_TOOL   ].validate(_("tool"),    v.sublocales[_("tool")   ]);
	errors += translations[LOCALE_CAT_TOOLTIP].validate(_("tooltip"), v.sublocales[_("tooltip")]);
	errors += translations[LOCALE_CAT_LABEL  ].validate(_("label"),   v.sublocales[_("label")  ]);
	errors += translations[LOCALE_CAT_BUTTON ].validate(_("button"),  v.sublocales[_("button") ]);
	errors += translations[LOCALE_CAT_TITLE  ].validate(_("title"),   v.sublocales[_("title")  ]);
	errors += translations[LOCALE_CAT_ACTION ].validate(_("action"),  v.sublocales[_("action") ]);
	errors += translations[LOCALE_CAT_ERROR  ].validate(_("error"),   v.sublocales[_("error")  ]);
	errors += translations[LOCALE_CAT_TYPE   ].validate(_("type"),    v.sublocales[_("type")   ]);
	// errors?
	if (!errors.empty()) {
		if (ver != app_version) {
			errors = _("Errors in locale file ") + short_name + _(":") + errors;
		} else {
			errors = _("Errors in locale file ") + short_name +
			         _("\nThis is probably because the locale was made for a different version of MSE.") + errors;
		}
		handle_warning(errors);
	}
}

String SubLocale::validate(const String& name, const SubLocaleValidatorP& v) const {
	if (!v) {
		return _("\nInternal error validating local file: expected keys file missing for \"") + name + _("\" section.");
	}
	String errors;
	// 1. keys in v but not in this, check arg count
	FOR_EACH_CONST(kc, v->keys) {
		map<String,String>::const_iterator it = translations.find(kc.first);
		if (it == translations.end()) {
			if (!kc.second.optional) {
				errors += _("\n   Missing key:\t\t\t") + name + _(": ") + kc.first;
			}
		} else if (string_format_args(it->second) != kc.second.args) {
			errors += _("\n   Incorrect number of arguments for:\t") + name + _(": ") + kc.first
			       +  String::Format(_("\t  expected: %d, found %d"), kc.second.args, string_format_args(it->second));
		}
	}
	// 2. keys in this but not in v
	FOR_EACH_CONST(kv, translations) {
		map<String,KeyValidator>::const_iterator it = v->keys.find(kv.first);
		if (it == v->keys.end() && !kv.second.empty()) {
			// allow extra keys with empty values as a kind of documentation
			// for example in the help stirngs:
			//   help:
			//       file:
			//       new set: blah blah
			errors += _("\n   Unexpected key:\t\t\t") + name + _(": ") + kv.first;
		}
	}
	return errors;
}