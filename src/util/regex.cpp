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

// -----------------------------------------------------------------------------
// : Regex : std

void Regex::assign(const String &code, std::basic_regex<Char>::flag_type flag) {
	// compile string
	m_empty = false;
	try {
		regex.assign(code.begin(), code.end(), flag);
	} catch (const std::regex_error &e) {
		/// TODO: be more precise
		throw ScriptError(String::Format(
			_("Error while compiling regular expression: '%s'\n%s"),
			code.c_str(),
			String(e.what(), IF_UNICODE(wxConvUTF8, String::npos)).c_str()));
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
