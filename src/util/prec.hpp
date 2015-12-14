//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_PREC
#define HEADER_UTIL_PREC

/** @file util/prec.hpp
 *
 *  @brief Precompiled header, and aliases for common types
 */

// Includes {{{

// Wx headers {{{
#include <wx/setup.h>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
// }}}

// STL headers {{{
#include <vector>
#include <map>
#include <set>
using std::vector;
using std::map;
using std::set;
// }}}

// Includes }}}

#undef RGB

// Wx Aliases {{{

// Remove some of the wxUglyness

typedef wxWindow Window;

typedef wxBitmap Bitmap;
typedef wxImage Image;
typedef wxColour Color;
typedef wxDC DC;

typedef wxDateTime DateTime;
// }}}

// Compatability fixes {{{

#if wxVERSION_NUMBER < 2900 && defined(__WXMSW__)
// see http://docs.wxwidgets.org/2.8.11/wx_wxmswport.html
#define wxBORDER_THEME_FIX(x)                                                  \
    (x & wxBORDER_THEME                                                        \
         ? (x & ~wxBORDER_THEME) | wxWindow::GetThemedBorderStyle()            \
         : x)
#else
#define wxBORDER_THEME_FIX(x) x
#endif
#if wxVERSION_NUMBER < 2900
// wx >= 2.9 requires the use of HandleWindowEvent on windows, instead of
// ProcessEvent
#define HandleWindowEvent ProcessEvent
#endif
#if wxVERSION_NUMBER < 2811
#define SetDeviceClippingRegion SetClippingRegion
#endif
// }}}

// Other aliases {{{
typedef unsigned char Byte;
typedef unsigned int UInt;
// }}}

// MSE Headers {{{
// MSE utility headers unlikely to change and used everywhere
#include "string.hpp"
#include "smart_ptr.hpp"
#include "index_map.hpp"
#include "locale.hpp"
#include "error.hpp"
#include "reflect.hpp"
// }}}

// Debugging fixes {{{

#if defined(_MSC_VER) && defined(_DEBUG) && defined(_CRT_WIDE)
// Use OutputDebugString/DebugBreak for assertions if in debug mode
void msvc_assert(const wchar_t *, const wchar_t *, const wchar_t *, unsigned);
#undef assert
#define assert(exp)                                                            \
    (void)((exp) || (msvc_assert(nullptr, _CRT_WIDE(#exp),                     \
                                 _CRT_WIDE(__FILE__), __LINE__),               \
                     0))
#endif

// }}}
#endif
