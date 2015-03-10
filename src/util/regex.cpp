//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// -----------------------------------------------------------------------------
// : Includes

#include <util/prec.hpp>
#include <util/regex.hpp>
#include <util/error.hpp>
#include <locale>

using std::basic_regex;
using std::regex_error;
using std::regex_replace;
using std::string;
using namespace std::regex_constants;

static String code_to_str(const regex_error &err) {
	// Taken from cplusplus.com
	switch (err.code()) {
	case error_collate:
		return _("Invalid collating element name");
	case error_ctype:
		return _("Invalid character class name");
	case error_escape:
		return _("Invalid escaped character, or a trailing escape");
	case error_backref:
		return _("Invalid back reference");
	case error_brack:
		return _("Mismatched brackets ([ and ])");
	case error_paren:
		return _("Mismatched brackets (( and ))");
	case error_brace:
		return _("Mismatched brackets ({ and })");
	case error_badbrace:
		return _("Invalid range between braces ({ and })");
	case error_range:
		return _("Invalid character range");
	case error_space:
		return _("Insufficient memory");
	case error_badrepeat:
		return _("Repeat specifier without valid regular expression");
	case error_complexity:
		return _("Too complex");
	case error_stack:
		return _("Insufficient memory to determine match");
	}
}

String escape_x(String text) {
	String ret;
	static std::locale loc;
	size_t depth{0};
	bool commented = false;
	for (size_t chr{0}; chr < text.length(); ++chr) {
		wchar_t c = text[chr];
		if (c == _('\n') && commented) {
			// End comment
			commented = false;
		} else if (c == _('#') && !depth) {
			// Start comment
			commented = true;
		} else if (c == _('[')) {
			// Depth in
			++depth;
		} else if (c == _(']')) {
			// Depth out
			if (!depth) {
				throw regex_error{error_brack};
			}
			--depth;
		} else if (c == _('\\')) {
			// Escaped character
			++chr;
			if (chr == text.length()) {
				throw regex_error{error_escape};
			}
			ret += text[chr];
		}
		if (!commented && (depth || !std::isspace(c, loc))) {
			ret += c;
		}
	}
	return ret;
}

// -----------------------------------------------------------------------------
// : Regex : std

void Regex::assign(const String &code, syntax_option_type flag) {
	// compile string
	m_empty = false;
	String prefix = _("(?");
	String icmp = _("(?i");
	String ispc = _("(?x");
	String text = code;
	try {
		while (text.StartsWith(prefix)) {
			if (text.StartsWith(icmp)) {
				flag |= icase;
				text.Replace(icmp, prefix, false);
			} else if (text.StartsWith(ispc)) {
				text.Replace(ispc, prefix, false);
				text = escape_x(text);
			} else {
				text.Replace(_("(?)"), _(""), false);
			}
		}
		regex.assign(text.begin(), text.end(), flag);
	} catch (const regex_error &e) {
		throw ScriptError(String::Format(
			_("Error while compiling regular expression: '%s'\n%s"),
			code.c_str(), code_to_str(e).c_str()));
	}
}

void Regex::replace_all(String *input, const String &format) {
	// std::basic_string<Char> fmt; format_string(format,fmt);
	std::basic_string<Char> fmt(format.begin(), format.end());
	String output;
	regex_replace(insert_iterator<String>(output, output.end()), input->begin(),
				  input->end(), regex, fmt, std::regex_constants::format_sed);
	*input = output;
}

// -----------------------------------------------------------------------------
// : Regex : common
