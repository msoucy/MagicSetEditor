//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/functions/functions.hpp>
#include <script/functions/util.hpp>
#include <util/tagged_string.hpp>
#include <util/error.hpp>

using std::min;

// ----------------------------------------------------------------------------- : Util

bool is_vowel(Char c) {
	return c == (L'a') || c == (L'e') || c == (L'i') || c == (L'o') || c == (L'u')
	    || c == (L'A') || c == (L'E') || c == (L'I') || c == (L'O') || c == (L'U');
}
inline bool is_constant(Char c) {
	return !is_vowel(c);
}

// ----------------------------------------------------------------------------- : Numbers

/// Write a number using words, for example 23 -> "twenty-three"
String english_number(int i) {
	switch (i) {
		case 0:  return (L"zero");
		case 1:  return (L"one");
		case 2:  return (L"two");
		case 3:  return (L"three");
		case 4:  return (L"four");
		case 5:  return (L"five");
		case 6:  return (L"six");
		case 7:  return (L"seven");
		case 8:  return (L"eight");
		case 9:  return (L"nine");
		case 10: return (L"ten");
		case 11: return (L"eleven");
		case 12: return (L"twelve");
		case 13: return (L"thirteen");
		case 15: return (L"fifteen");
		case 18: return (L"eighteen");
		case 20: return (L"twenty");
		case 30: return (L"thirty");
		case 40: return (L"forty");
		case 50: return (L"fifty");
		case 80: return (L"eighty");
		default: {
			if (i < 0 || i >= 100) {
				// number too large, keep as digits
				return (String() << i);
			} else if (i < 20) {
				return english_number(i%10) + (L"teen");
			} else if (i % 10 == 0) {
				return english_number(i/10) + (L"ty");
			} else {
				// <a>ty-<b>
				return english_number(i/10*10) + (L"-") + english_number(i%10);
			}
		}
	}
}

/// Write an ordinal number using words, for example 23 -> "twenty-third"
String english_ordinal(int i) {
	switch (i) {
		case 1:  return (L"first");
		case 2:  return (L"second");
		case 3:  return (L"third");
		case 5:  return (L"fifth");
		case 8:  return (L"eighth");
		case 9:  return (L"ninth");
		case 11: return (L"eleventh");
		case 12: return (L"twelfth");
		default: {
			if (i <= 0 || i >= 100) {
				// number too large, keep as digits
				return String::Format((L"%dth"), i);
			} else if (i < 20) {
				return english_number(i) + (L"th");
			} else if (i % 10 == 0) {
				String num = english_number(i);
				return num.substr(0,num.size()-1) + (L"ieth"); // twentieth
			} else {
				// <a>ty-<b>th
				return english_number(i/10*10) + (L"-") + english_ordinal(i%10);
			}
		}
	}
}

/// Write a number using words, use "a" for 1
String english_number_a(int i) {
	if (i == 1) return (L"a");
	else        return english_number(i);
}
/// Write a number using words, use "" for 1
String english_number_multiple(int i) {
	if (i == 1) return (L"");
	else        return english_number(i);
}


// script_english_number_*
String do_english_num(String input, String(*fun)(int)) {
	if (is_substr(input, 0, (L"<param-"))) {
		// a keyword parameter, of the form "<param->123</param->"
		size_t start = skip_tag(input, 0);
		if (start != String::npos) {
			size_t end = input.find_first_of((L'<'), start);
			if (end != String::npos) {
				String is = input.substr(start, end - start);
				long i = 0;
				if (is.ToLong(&i)) {
					if (i == 1) {
						return (L"<hint-1>") + substr_replace(input, start, end, fun(i));
					} else {
						return (L"<hint-2>") + substr_replace(input, start, end, fun(i));
					}
				}
			}
		}
		return (L"<hint-2>") + input;
	} else {
		long i = 0;
		if (input.ToLong(&i)) {
			return fun(i);
		}
		return input;
	}
}

SCRIPT_FUNCTION(english_number) {
	SCRIPT_PARAM_C(String, input);
	SCRIPT_RETURN(do_english_num(input, english_number));
}
SCRIPT_FUNCTION(english_number_a) {
	SCRIPT_PARAM_C(String, input);
	SCRIPT_RETURN(do_english_num(input, english_number_a));
}
SCRIPT_FUNCTION(english_number_multiple) {
	SCRIPT_PARAM_C(String, input);
	SCRIPT_RETURN(do_english_num(input, english_number_multiple));
}
SCRIPT_FUNCTION(english_number_ordinal) {
	SCRIPT_PARAM_C(String, input);
	SCRIPT_RETURN(do_english_num(input, english_ordinal));
}

// ----------------------------------------------------------------------------- : Singular/plural

String english_singular(const String& str) {
	if (str.size() > 3 && is_substr(str, str.size()-3, (L"ies"))) {
		return str.substr(0, str.size() - 3) + (L"y");
	} else if (str.size() > 3 && is_substr(str, str.size()-3, (L"oes"))) {
		return str.substr(0, str.size() - 2);
	} else if (str.size() > 4 && is_substr(str, str.size()-4, (L"ches"))) {
		return str.substr(0, str.size() - 2);
	} else if (str.size() > 4 && is_substr(str, str.size()-4, (L"shes"))) {
		return str.substr(0, str.size() - 2);
	} else if (str.size() > 4 && is_substr(str, str.size()-4, (L"sses"))) {
		return str.substr(0, str.size() - 2);
	} else if (str.size() > 5 && is_substr(str, str.size()-3, (L"ves")) && (is_substr(str, str.size()-5, (L"el")) || is_substr(str, str.size()-5, (L"ar")) )) {
		return str.substr(0, str.size() - 3) + (L"f");
	} else if (str.size() > 1 && str.GetChar(str.size() - 1) == (L's')) {
		return str.substr(0, str.size() - 1);
	} else if (str.size() >= 3 && is_substr(str, str.size()-3, (L"men"))) {
		return str.substr(0, str.size() - 2) + (L"an");
	} else {
		return str;
	}
}
String english_plural(const String& str) {
	if (str.size() > 2) {
		Char a = str.GetChar(str.size() - 2);
		Char b = str.GetChar(str.size() - 1);
		if (b == (L'y') && is_constant(a)) {
			return str.substr(0, str.size() - 1) + (L"ies");
		} else if (b == (L'o') && is_constant(a)) {
			return str + (L"es");
		} else if ((a == (L's') || a == (L'c')) && b == (L'h')) {
			return str + (L"es");
		} else if (b == (L's')) {
			return str + (L"es");
		} else {
			return str + (L"s");
		}
	}
	return str + (L"s");
}

// script_english_singular/plural/singplur
String do_english(String input, String(*fun)(const String&)) {
	if (is_substr(input, 0, (L"<param-"))) {
		// a keyword parameter, of the form "<param->123</param->"
		size_t start = skip_tag(input, 0);
		if (start != String::npos) {
			size_t end = input.find_first_of((L'<'), start);
			if (end != String::npos) {
				String is = input.substr(start, end - start);
				return substr_replace(input, start, end, fun(is));
			}
		}
		return input; // failed
	} else {
		return fun(input);
	}
}

SCRIPT_FUNCTION(english_singular) {
	SCRIPT_PARAM_C(String, input);
	SCRIPT_RETURN(do_english(input, english_singular));
}
SCRIPT_FUNCTION(english_plural) {
	SCRIPT_PARAM_C(String, input);
	SCRIPT_RETURN(do_english(input, english_plural));
}

// ----------------------------------------------------------------------------- : Hints

/// Process english hints in the input string
/** A hint is formed by
 *    1. an insertion of a parameter, <param-..>...</param->.
 *    2. a <hint-1> or <hint-2> tag
 *   
 *  Hints have the following meaning:
 *   -  "<hint-1>xxx(yyy)zzz"       ---> "xxxzzz"     (singular)
 *   -  "<hint-2>xxx(yyy)zzz"       ---> "xxxyyyzzz"  (plural)
 *   -  "[^., ]a <param-..>[aeiou]" ---> "\1 an \2"   (articla 'an', case insensitive)
 *   -  "<hint-?>"                  ---> ""           (remove <hint>s afterwards)
 *
 *  Note: there is no close tags for hints
 */
String process_english_hints(const String& str) {
	String ret; ret.reserve(str.size());
	// have we seen a <hint-1/2>?
	// 1 for singular, 2 for plural
	int singplur = 0;
	for (size_t i = 0 ; i < str.size() ; ) {
		Char c = str.GetChar(i);
		if (is_substr(str, i, (L"<hint-"))) {
			Char h = str.GetChar(i + 6); // hint code
			if (h == (L'1')) {
				singplur = 1;
			} else if (h == (L'2')) {
				singplur = 2;
			}
			i = skip_tag(str, i);
		} else if (is_substr(str, i, (L"<param-"))) {
			size_t after = skip_tag(str, i);
			if (after != String::npos) {
				Char c = str.GetChar(after);
				if (singplur == 1 && is_substr(str, after, (L"a</param-"))) {
					// a -> an, where the a originates from english_number_a(1)
					size_t after_end = skip_tag(str,after+1);
					if (after_end == String::npos) {
						throw Error((L"Incomplete </param> tag"));
					}
					if (after_end != String::npos && after_end + 1 < str.size()
					&& isSpace(str.GetChar(after_end)) && is_vowel(str.GetChar(after_end+1))) {
						ret.append(str,i,after-i);
						ret += (L"an");
						i = after + 1;
						continue;
					}
				} else if (is_vowel(c) && ret.size() >= 2) {
					// a -> an?
					// is there "a" before this?
					String last = ret.substr(ret.size() - 2);
					if ( (ret.size() == 2 || !isAlpha(ret.GetChar(ret.size() - 3))) &&
						(last == (L"a ") || last == (L"A ")) ) {
						ret.insert(ret.size() - 1, (L'n'));
					}
				} else if (is_substr(str, after, (L"</param-")) && ret.size() >= 1 &&
							ret.GetChar(ret.size() - 1) == (L' ')) {
					// empty param, drop space before it
					ret.resize(ret.size() - 1);
				}
			}
			ret.append(str,i,min(after,str.size())-i);
			i = after;
		} else if (is_substr(str, i, (L"<singular>"))) {
			// singular -> keep, plural -> drop
			size_t start = skip_tag(str, i);
			size_t end   = match_close_tag(str, start);
			if (singplur == 1 && end != String::npos) {
				ret += str.substr(start, end - start);
			}
			singplur = 0;
			i = skip_tag(str, end);
		} else if (is_substr(str, i, (L"<plural>"))) {
			// singular -> drop, plural -> keep
			size_t start = skip_tag(str, i);
			size_t end   = match_close_tag(str, start);
			if (singplur == 2 && end != String::npos) {
				ret += str.substr(start, end - start);
			}
			singplur = 0;
			i = skip_tag(str, end);
		} else if (c == (L'(') && singplur) {
			// singular -> drop (...), plural -> keep it
			size_t end = str.find_first_of((L')'), i);
			if (end != String::npos) {
				if (singplur == 2) {
					ret += str.substr(i + 1, end - i - 1);
				}
				i = end + 1;
			} else { // handle like normal
				ret += c;
				++i;
			}
			singplur = 0;
		} else if (c == (L'<')) {
			size_t after = skip_tag(str, i);
			ret.append(str,i,min(after,str.size())-i);
			i = after;
		} else {
			ret += c;
			++i;
		}
	}
	return ret;
}

SCRIPT_FUNCTION(process_english_hints) {
	SCRIPT_PARAM_C(String, input);
	assert_tagged(input);
	SCRIPT_RETURN(process_english_hints(input));
}

// ----------------------------------------------------------------------------- : Init

void init_script_english_functions(Context& ctx) {
	ctx.setVariable((L"english_number"),          script_english_number);
	ctx.setVariable((L"english_number_a"),        script_english_number_a);
	ctx.setVariable((L"english_number_multiple"), script_english_number_multiple);
	ctx.setVariable((L"english_number_ordinal"),  script_english_number_ordinal);
	ctx.setVariable((L"english_singular"),        script_english_singular);
	ctx.setVariable((L"english_plural"),          script_english_plural);
	ctx.setVariable((L"process_english_hints"),   script_process_english_hints);
}
