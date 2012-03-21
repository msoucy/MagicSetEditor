//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_AGE
#define HEADER_UTIL_AGE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/dynamic_arg.hpp>
#include <util/atomic.hpp>

// ----------------------------------------------------------------------------- : Age

/// Represents the age of a value, higher values are newer
/** Age is counted using a global variable */
class Age {
  public:
	/// Default constructor: gives 'start of time'
	Age() : age(0) {}
	Age(AtomicIntEquiv a) : age(a) {}
	
	/// Get the next age, larger than all previous ages
	inline static Age next() {
		return Age(++new_age);
	}
	/// Get the current age, without incrementing the global age counter
	inline static Age current() {
		return Age(new_age);
	}
	
	/// Compare two ages, smaller means earlier
	inline bool operator <  (Age a) const { return age <  a.age; }
	inline bool operator <= (Age a) const { return age <= a.age; }
	inline bool operator >  (Age a) const { return age >  a.age; }
	inline bool operator >= (Age a) const { return age >= a.age; }
	inline bool operator == (Age a) const { return age == a.age; }
	
	/// A number corresponding to the age
	inline AtomicIntEquiv get() const { return age; }
	
  private:
	/// This age
	AtomicIntEquiv age;
	/// Global age counter, value of the last age created
	static AtomicInt new_age;
};


/// Age the object currently being processed was last updated
DECLARE_DYNAMIC_ARG(AtomicIntEquiv, last_update_age);

// ----------------------------------------------------------------------------- : EOF
#endif
