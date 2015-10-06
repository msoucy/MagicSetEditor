//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_FOR_EACH
#define HEADER_UTIL_FOR_EACH

/** @file util/for_each.hpp
 *
 *  @brief Macros to simplify looping over collections.
 * 
 *  This header contains some evil template and macro hackery.
 */

// ----------------------------------------------------------------------------- : Includes

// ----------------------------------------------------------------------------- : Typeof magic

#ifdef __GNUC__
	// GCC has a buildin typeof function, so it doesn't need (as much) hacks
	#define DECLARE_TYPEOF(T)
	#define DECLARE_TYPEOF_NO_REV(T)
	#define DECLARE_TYPEOF_CONST(T)
	#define DECLARE_TYPEOF_COLLECTION(T)
	
	#define TYPEOF(Value)      __typeof(Value)
	#define TYPEOF_IT(Value)   __typeof((Value).begin())
	#define TYPEOF_CIT(Value)  __typeof((Value).begin())
	#define TYPEOF_RIT(Value)  __typeof((Value).rbegin())
	#define TYPEOF_CRIT(Value) __typeof((Value).rbegin())
	#define TYPEOF_REF(Value)  __typeof(*(Value).begin())&
	#define TYPEOF_CREF(Value) __typeof(*(Value).begin())&
	
#else
	/// Helper for typeof tricks
	template<const type_info &ref_type_info> struct TypeOf {};

	/// The type of a value
	#define TYPEOF(Value)      TypeOf<typeid(Value)>::type
	/// The type of an iterator
	#define TYPEOF_IT(Value)   TypeOf<typeid(Value)>::iterator
	/// The type of a const iterator
	#define TYPEOF_CIT(Value)  TypeOf<typeid(Value)>::const_iterator
	/// The type of a reverse iterator
	#define TYPEOF_RIT(Value)  TypeOf<typeid(Value)>::reverse_iterator
	/// The type of a const reverse iterator
	#define TYPEOF_CRIT(Value) TypeOf<typeid(Value)>::const_reverse_iterator
	/// The type of a reference
	#define TYPEOF_REF(Value)  TypeOf<typeid(Value)>::reference
	/// The type of a const reference
	#define TYPEOF_CREF(Value) TypeOf<typeid(Value)>::const_reference
	
	/// Declare typeof magic for a specific type
	#define DECLARE_TYPEOF(T)                                           \
	    template<> struct TypeOf<typeid(T)> {                           \
	        typedef T               type;                               \
	        typedef T::iterator         iterator;                       \
	        typedef T::const_iterator       const_iterator;             \
	        typedef T::reverse_iterator     reverse_iterator;           \
	        typedef T::const_reverse_iterator   const_reverse_iterator; \
	        typedef T::reference            reference;                  \
	        typedef T::const_reference      const_reference;            \
	    }
	/// Declare typeof magic for a specific type that doesn't support reverse iterators
	#define DECLARE_TYPEOF_NO_REV(T)                                    \
	    template<> struct TypeOf<typeid(T)> {                           \
	        typedef T               type;                               \
	        typedef T::iterator         iterator;                       \
	        typedef T::const_iterator       const_iterator;             \
	        typedef T::reference            reference;                  \
	        typedef T::const_reference      const_reference;            \
	    }
	/// Declare typeof magic for a specific type, using const iterators
	#define DECLARE_TYPEOF_CONST(T)                                     \
	    template<> struct TypeOf<typeid(T)> {                           \
	        typedef T               type;                               \
	        typedef T::const_iterator       iterator;                   \
	        typedef T::const_iterator       const_iterator;             \
	        typedef T::const_reverse_iterator   reverse_iterator;       \
	        typedef T::const_reverse_iterator   const_reverse_iterator; \
	        typedef T::const_reference      reference;                  \
	        typedef T::const_reference      const_reference;            \
	    }
	
	/// Declare typeof magic for a specific std::vector type
	#define DECLARE_TYPEOF_COLLECTION(T)  DECLARE_TYPEOF(vector< T >);  \
	                                      DECLARE_TYPEOF_CONST(set< T >)
	
#endif

/// Use for template classes
/** i.e.
 *    DECLARE_TYPEOF(pair<a COMMA b>);
 *  instead of
 *    DECLARE_TYPEOF(pair<a,b>);
 */
#define COMMA ,

// ----------------------------------------------------------------------------- : EOF
#endif
