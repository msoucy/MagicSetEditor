//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_REGEX
#define HEADER_UTIL_REGEX

// -----------------------------------------------------------------------------
// : Includes

#include <regex>
#include <util/prec.hpp>

// -----------------------------------------------------------------------------
// : STL implementation

class Regex {
  public:
	struct Results : public std::match_results<String::const_iterator> {
		/// Get a sub match
		inline String str(int sub = 0) const {
			const_reference v = (*this)[sub];
			return String(v.first, v.second);
		}
		/// Format a replacement string
		inline String format(const String &format) const {
			std::basic_string<Char> fmt(format.begin(), format.end());
			String output;
			std::match_results<String::const_iterator>::format(
				insert_iterator<String>(output, output.end()), fmt,
				std::regex_constants::format_sed);
			return output;
		}
	};

	inline Regex() {}
	inline Regex(const String &code) { assign(code); }

	void assign(const String &code,
				std::basic_regex<Char>::flag_type = std::regex::ECMAScript);
	inline bool matches(const String &str) const {
		return regex_search(str.begin(), str.end(), regex);
	}
	inline bool matches(Results &results, const String &str,
						size_t start = 0) const {
		return matches(results, str.begin() + start, str.end());
	}
	inline bool matches(Results &results, const String::const_iterator &begin,
						const String::const_iterator &end) const {
		return regex_search(begin, end, results, regex);
	}
	void replace_all(String *input, const String &format);

	// This will always be false
	// Failed assignment throws
	inline bool empty() const { return m_empty; }

  private:
	std::basic_regex<Char> regex; ///< The regular expression
	bool m_empty = true;
};

// -----------------------------------------------------------------------------
// : EOF
#endif
