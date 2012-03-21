//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_SCRIPT_VALUE
#define HEADER_SCRIPT_VALUE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gfx/color.hpp>
class Context;
class Dependency;
class ScriptClosure;
DECLARE_POINTER_TYPE(GeneratedImage);

// ----------------------------------------------------------------------------- : ScriptValue

DECLARE_POINTER_TYPE(ScriptValue);

enum ScriptType
{	SCRIPT_NIL
,	SCRIPT_INT
,	SCRIPT_BOOL
,	SCRIPT_DOUBLE
,	SCRIPT_STRING
,	SCRIPT_COLOR
,	SCRIPT_IMAGE
,	SCRIPT_FUNCTION
,	SCRIPT_OBJECT // Only ScriptObject
,	SCRIPT_COLLECTION
,	SCRIPT_REGEX
,	SCRIPT_DATETIME
,	SCRIPT_ITERATOR
,	SCRIPT_FILENAME
,	SCRIPT_DUMMY
,	SCRIPT_ERROR
};

enum CompareWhat
{	COMPARE_NO
,	COMPARE_AS_STRING
,	COMPARE_AS_POINTER
};

/// A value that can be handled by the scripting engine.
/// Actual values are derived types
class ScriptValue : public IntrusivePtrBaseWithDelete {
  public:
	virtual ~ScriptValue() {}

	/// Information on the type of this value
	virtual ScriptType type() const = 0;
	/// Name of the type of value
	virtual String typeName() const = 0;
	/// How to compare this object?
	/** Returns 1 if the pointer should be used, 0 otherwise */
	virtual CompareWhat compareAs(String& compare_str, void const*& compare_ptr) const;

	/// Convert this value to a string
	virtual String toString() const;
	/// Script code to generate this value
	virtual String toCode() const;
	/// A string that can be shown to a user (i.e. not a template developer), should not contains tags
	virtual String toFriendlyString() const;
	/// Convert this value to a double
	virtual double toDouble() const;
	/// Convert this value to an integer
	virtual int toInt() const;
	/// Convert this value to a boolean
	virtual bool toBool() const;
	/// Convert this value to a color
	virtual AColor toColor() const;
	/// Convert this value to a wxDateTime
	virtual wxDateTime toDateTime() const;
	/// Convert this value to an image
	virtual GeneratedImageP toImage() const;
	/// Is the value equal to script_nil?
	inline bool isNil() const { return type() == SCRIPT_NIL; }
	
	/// Get a member variable from this value
	virtual ScriptValueP getMember(const String& name) const;

	/// Signal that a script depends on this value itself
	virtual void dependencyThis(const Dependency& dep);
	/// Signal that a script depends on a member of this value
	/** It is the abstract version of getMember*/
	virtual ScriptValueP dependencyMember(const String& name, const Dependency&) const;
	/// Signal that a script depends on a member of container, with the name of this
	/** This function allows for a kind of visitor pattern over dependencyMember */
	virtual ScriptValueP dependencyName(const ScriptValue& container, const Dependency&) const;

	/// Evaluate this value (if it is a function)
	ScriptValueP eval(Context& ctx, bool openScope = true) const {
		return do_eval(ctx, openScope);
	}
	/// Mark the scripts that this function depends on
	/** Return value is an abstract version of the return value of eval */
	virtual ScriptValueP dependencies(Context&, const Dependency&) const;
	/// Simplify/optimize a default argument closure of this function.
	/** Should return a simplification of the closure or null to keep the closure.
	 *  Alternatively, the closure may be modified in place.
	 */
	virtual ScriptValueP simplifyClosure(ScriptClosure&) const;

	/// Return an iterator for the current collection, an iterator is a value that has next()
	virtual ScriptValueP makeIterator() const;
	/// Return the next item for this iterator, or ScriptValueP() if there is no such item
	/** If key_out != 0, then it will recieve the key of the item */
	virtual ScriptValueP next(ScriptValueP* key_out = nullptr);
	/// Return the number of items in this value (assuming it is a collection)
	virtual int itemCount() const;
	/// Get a member at the given index
	virtual ScriptValueP getIndex(int index) const;

  protected:
	virtual ScriptValueP do_eval(Context& ctx, bool openScope) const;
};

extern ScriptValueP script_nil;   ///< The preallocated nil value
extern ScriptValueP script_true;  ///< The preallocated true value
extern ScriptValueP script_false; ///< The preallocated false value
extern ScriptValueP dependency_dummy; ///< Dummy value used during dependency analysis

/// compare script values for equallity
bool equal(const ScriptValueP& a, const ScriptValueP& b);

// ----------------------------------------------------------------------------- : EOF
#endif
