//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_FOR_EACH
#define HEADER_UTIL_FOR_EACH

/** @file util/for_each.hpp
 *
 *  @brief Includes to simplify looping over collections.
 */

// -----------------------------------------------------------------------------
// : Includes

#include <boost/range/combine.hpp>
#include <boost/range/adaptor/reversed.hpp>
using boost::combine;
using boost::adaptors::reverse;

// -----------------------------------------------------------------------------
// : EOF
#endif
