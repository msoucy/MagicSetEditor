//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_ATOMIC
#define HEADER_UTIL_ATOMIC

/** @file util/atomic.hpp
 *
 *  @brief Provides the type AtomicInt, which is an integer that can be
 *incremented and decremented atomicly
 */

// -----------------------------------------------------------------------------
// : Includes

#include <atomic>

/// We have a fast AtomicInt
#define HAVE_FAST_ATOMIC

/// An integer which is equivalent to an AtomicInt, but which doesn't support
/// attomic operations
typedef long AtomicIntEquiv;

/// An integer that can be incremented and decremented atomicly
typedef std::atomic<AtomicIntEquiv> AtomicInt;

// -----------------------------------------------------------------------------
// : EOF
#endif
