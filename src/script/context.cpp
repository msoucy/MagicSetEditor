//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/context.hpp>
#include <script/to_value.hpp>
#include <util/error.hpp>
#include <iostream>

// ----------------------------------------------------------------------------- : Context

Context::Context()
	: level(0)
{}

// ----------------------------------------------------------------------------- : Evaluate

// Perform a unary simple instruction, store the result in a (not in *a)
void instrUnary  (UnaryInstructionType   i, ScriptValueP& a);

// Perform a binary simple instruction, store the result in a (not in *a)
void instrBinary (BinaryInstructionType  i, ScriptValueP& a, const ScriptValueP& b);

// Perform a ternary simple instruction, store the result in a (not in *a)
void instrTernary(TernaryInstructionType i, ScriptValueP& a, const ScriptValueP& b, const ScriptValueP& c);

// Perform a quaternary simple instruction, store the result in a (not in *a)
void instrQuaternary(QuaternaryInstructionType i, ScriptValueP& a, const ScriptValueP& b, const ScriptValueP& c, const ScriptValueP& d);


ScriptValueP Context::eval(const Script& script, bool useScope) {
	if (level > 500) {
		throw ScriptError(_("Stack overflow"));
	}
	
	size_t stack_size = stack.size();
	size_t scope = useScope ? openScope() : 0;
	try {
		// Instruction pointer
		const Instruction* instr = &script.instructions[0];
		const Instruction* end   = &*script.instructions.end();
		
		// Loop until we are done
		while (instr < end) {
			// Evaluate the current instruction
			Instruction i = *instr++;
			switch (i.instr) {
				case I_NOP: break;
				// Push a constant
				case I_PUSH_CONST: {
					stack.push_back(script.constants[i.data]);
					break;
				}
				// Jump
				case I_JUMP: {
					instr = &script.instructions[i.data];
					break;
				}
				// Conditional jump
				case I_JUMP_IF_NOT: {
					bool condition = *stack.back();
					stack.pop_back();
					if (!condition) {
						instr = &script.instructions[i.data];
					}
					break;
				}
				
				// Get a variable
				case I_GET_VAR: {
					ScriptValueP value = variables[i.data].value;
					if (!value) throw ScriptError(_("Variable not set: ") + variable_to_string((Variable)i.data));
					stack.push_back(value);
					break;
				}
				// Set a variable
				case I_SET_VAR: {
					setVariable((Variable)i.data, stack.back());
					break;
				}
				
				// Get an object member
				case I_MEMBER_C: {
					stack.back() = stack.back()->getMember(*script.constants[i.data]);
					break;
				}
				// Loop over a container, push next value or jump
				case I_LOOP: {
					ScriptValueP& it = stack[stack.size() - 2]; // second element of stack
					ScriptValueP val = it->next();
					if (val) {
						stack.push_back(val);
					} else {
						stack.erase(stack.end() - 2); // remove iterator
						instr = &script.instructions[i.data];
					}
					break;
				}
				// Make an object
				case I_MAKE_OBJECT: {
					makeObject(i.data);
					break;
				}
				
				// Function call
				case I_CALL: {
					// new scope
					size_t scope = openScope();
					// prepare arguments
					for (unsigned int j = 0 ; j < i.data ; ++j) {
						setVariable((Variable)instr[i.data - j - 1].data, stack.back());
						stack.pop_back();
					}
					instr += i.data; // skip arguments
					try {
						// get function and call
						stack.back() = stack.back()->eval(*this);
					} catch (const Error& e) {
						// try to determine what named function was called
						// the instructions for this look like:
						//   I_GET_VAR   name of function
						//   *code*      arguments
						//   I_CALL      number of arguments = i.data
						//   I_NOP * n   arg names
						//   next        <--- instruction pointer points here
						// skip the stack effect of the arguments themselfs
						const Instruction* instr_bt = script.backtraceSkip(instr - i.data - 2, i.data);
						// have we have reached the name
						if (instr_bt) {
							throw ScriptError(_ERROR_2_("in function", e.what(), script.instructionName(instr_bt)));
						} else {
							throw e; // rethrow
						}
					}
					// restore scope
					closeScope(scope);
					break;
				}
				
				// Closure object
				case I_CLOSURE: {
					makeClosure(i.data, instr);
					break;
				}
				
				// Simple instruction: unary
				case I_UNARY: {
					instrUnary(i.instr1, stack.back());
					break;
				}
				// Simple instruction: binary
				case I_BINARY: {
					ScriptValueP  b = stack.back(); stack.pop_back();
					ScriptValueP& a = stack.back();
					instrBinary(i.instr2, a, b);
					break;
				}
				// Simple instruction: ternary
				case I_TERNARY: {
					ScriptValueP  c = stack.back(); stack.pop_back();
					ScriptValueP  b = stack.back(); stack.pop_back();
					ScriptValueP& a = stack.back();
					instrTernary(i.instr3, a, b, c);
					break;
				}
				// Simple instruction: quaternary
				case I_QUATERNARY: {
					ScriptValueP  d = stack.back(); stack.pop_back();
					ScriptValueP  c = stack.back(); stack.pop_back();
					ScriptValueP  b = stack.back(); stack.pop_back();
					ScriptValueP& a = stack.back();
					instrQuaternary(i.instr4, a, b, c, d);
					break;
				}
				// Duplicate stack
				case I_DUP: {
					stack.push_back(stack.at(stack.size() - i.data - 1));
					break;
				}
			}
		}
		
		// Function return
		// restore shadowed variables
		if (useScope) closeScope(scope);
		// return top of stack
		ScriptValueP result = stack.back();
		stack.pop_back();
		assert(stack.size() == stack_size); // we end up with the same stack
		return result;
		
	} catch (...) {
		// cleanup after an exception
		if (useScope) closeScope(scope); // restore scope
		stack.resize(stack_size);     // restore stack
		throw; // rethrow
	}
}

void Context::setVariable(const String& name, const ScriptValueP& value) {
	setVariable(string_to_variable(name), value);
}

#ifdef _DEBUG
	extern vector<String> variable_names;
#endif

void Context::setVariable(Variable name, const ScriptValueP& value) {
	#ifdef _DEBUG
		assert((size_t)name < variable_names.size());
	#endif
	VariableValue& var = variables[name];
	if (var.level < level) {
		// keep shadow copy
		Binding bind = {name, var};
		shadowed.push_back(bind);
	}
	var.level = level;
	var.value = value;
}

ScriptValueP Context::getVariable(const String& name) {
	ScriptValueP value = variables[string_to_variable(name)].value;
	if (!value) throw ScriptError(_("Variable not set: ") + name);
	return value;
}

ScriptValueP Context::getVariableOpt(const String& name) {
	return variables[string_to_variable(name)].value;
}
ScriptValueP Context::getVariable(Variable var) {
	if (variables[var].value) return variables[var].value;
	throw ScriptError(_("Variable not set: ") + variable_to_string(var));
}
ScriptValueP Context::getVariableInScopeOpt(Variable var) {
	if (variables[var].level == level) return variables[var].value;
	else                               return ScriptValueP();
}
int Context::getVariableScope(Variable var) {
	if (variables[var].value) return level - variables[var].level;
	else                      return -1;
}

ScriptValueP Context::makeClosure(const ScriptValueP& fun) {
	intrusive_ptr<ScriptClosure> closure(new ScriptClosure(fun));
	// we can find out which variables are in the last level by looking at shadowed
	// these variables will be at the end of the list
	for (size_t i = shadowed.size() - 1 ; i + 1 > 0 ; --i) {
		Variable var = shadowed[i].variable;
		assert(variables[var].value);
		if (variables[var].level < level) break;
		closure->addBinding(var, variables[var].value);
	}
	// can we simplify?
	ScriptValueP better = closure->simplify();
	if (better) return better;
	else        return closure;
}


size_t Context::openScope() {
	level += 1;
	return shadowed.size();
}
void Context::closeScope(size_t scope) {
	assert(level > 0);
	assert(scope <= shadowed.size());
	level -= 1;
	// restore shadowed variables
	while (shadowed.size() > scope) {
		variables[shadowed.back().variable] = shadowed.back().value;
		shadowed.pop_back();
	}
}

// ----------------------------------------------------------------------------- : Simple instructions : unary

void instrUnary  (UnaryInstructionType   i, ScriptValueP& a) {
	switch (i) {
		case I_ITERATOR_C:
			a = a->makeIterator(a);
			break;
		case I_NEGATE: {
			ScriptType at = a->type();
			if (at == SCRIPT_DOUBLE) {
				a = to_script(-(double)*a);
			} else {
				a = to_script(-(int)*a);
			}
			break;
		} case I_NOT:
			a = to_script(!(bool)*a);
			break;
	}
}

// ----------------------------------------------------------------------------- : Function composition

/// Composition of two functions
class ScriptCompose : public ScriptValue {
  public:
	ScriptCompose(ScriptValueP a, ScriptValueP b) : a(a), b(b) {}
	
	virtual ScriptType type() const { return SCRIPT_FUNCTION; }
	virtual String typeName() const { return _("function composition"); }
	virtual ScriptValueP eval(Context& ctx) const {
		ctx.setVariable(SCRIPT_VAR_input, a->eval(ctx));
		return b->eval(ctx);
	}
	virtual ScriptValueP dependencies(Context& ctx, const Dependency& dep) const {
		ctx.setVariable(SCRIPT_VAR_input, a->dependencies(ctx, dep));
		return b->dependencies(ctx, dep);
	}
  private:
	ScriptValueP a,b;
};

// ----------------------------------------------------------------------------- : Simple instructions : binary

// operator on ints
#define OPERATOR_I(OP)											\
	a = to_script((int)*a  OP  (int)*b);						\
	break

// operator on bools
#define OPERATOR_B(OP)											\
	a = to_script((bool)*a  OP  (bool)*b);						\
	break

// operator on doubles or ints
#define OPERATOR_DI(OP)											\
	if (at == SCRIPT_DOUBLE || bt == SCRIPT_DOUBLE) {			\
		a = to_script((double)*a  OP  (double)*b);				\
	} else {													\
		a = to_script((int)*a     OP  (int)*b);					\
	}															\
	break

// operator on doubles or ints, defined as a function
#define OPERATOR_FUN_DI(OP)										\
	if (at == SCRIPT_DOUBLE || bt == SCRIPT_DOUBLE) {			\
		a = to_script(OP((double)*a,  (double)*b));				\
	} else {													\
		a = to_script(OP((int)*a,     (int)*b));				\
	}															\
	break


void instrBinary (BinaryInstructionType  i, ScriptValueP& a, const ScriptValueP& b) {
	switch (i) {
		case I_POP:
			// a = a;
			break;
		case I_MEMBER:
			a = a->getMember(*b);
			break;
		case I_ITERATOR_R:
			a = rangeIterator(*a, *b);
			break;
		default:
	  ScriptType at = a->type(), bt = b->type();
	  switch(i) {
		case I_ADD: // add is quite overloaded
			if (at == SCRIPT_NIL) {
				a = b;
			} else if (bt == SCRIPT_NIL) {
				// a = a;
			} else if (at == SCRIPT_FUNCTION && bt == SCRIPT_FUNCTION) {
				a = new_intrusive2<ScriptCompose>(a, b);
			} else if (at == SCRIPT_COLLECTION && bt == SCRIPT_COLLECTION) {
				a = new_intrusive2<ScriptConcatCollection>(a, b);
			} else if (at == SCRIPT_INT    && bt == SCRIPT_INT) {
				a = to_script((int)*a        +  (int)*b);
			} else if ((at == SCRIPT_INT || at == SCRIPT_DOUBLE) &&
			           (bt == SCRIPT_INT || bt == SCRIPT_DOUBLE)) {
				a = to_script((double)*a     +  (double)*b);
			} else {
				a = to_script(a->toString()  +  b->toString());
			}
			break;
		case I_SUB:		OPERATOR_DI(-);
		case I_MUL:		OPERATOR_DI(*);
		case I_FDIV:
			a = to_script((double)*a / (double)*b);
			break;
		case I_DIV:
			if (at == SCRIPT_DOUBLE || bt == SCRIPT_DOUBLE) {
				a = to_script((int)((double)*a / (double)*b));
			} else {
				a = to_script((int)*a / (int)*b);
			}
			break;
		case I_MOD:
			if (at == SCRIPT_DOUBLE || bt == SCRIPT_DOUBLE) {
				a = to_script(fmod((double)*a, (double)*b));
			} else {
				a = to_script((int)*a % (int)*b);
			}
			break;
		case I_AND:		OPERATOR_B(&&);
		case I_OR:		OPERATOR_B(||);
		case I_XOR:		OPERATOR_B(!=);
		case I_EQ:		a = to_script( equal(a,b));  break;
		case I_NEQ:		a = to_script(!equal(a,b));  break;
		case I_LT:		OPERATOR_DI(<);
		case I_GT:		OPERATOR_DI(>);
		case I_LE:		OPERATOR_DI(<=);
		case I_GE:		OPERATOR_DI(>=);
		case I_MIN:		OPERATOR_FUN_DI(min);
		case I_MAX:		OPERATOR_FUN_DI(max);
		case I_OR_ELSE:
			if (at == SCRIPT_ERROR) a = b;
			break;
	}}
}

// ----------------------------------------------------------------------------- : Simple instructions : ternary

void instrTernary(TernaryInstructionType i, ScriptValueP& a, const ScriptValueP& b, const ScriptValueP& c) {
	switch (i) {
		case I_RGB:
			a = to_script(Color((int)*a, (int)*b, (int)*c));
			break;
	}
}

// ----------------------------------------------------------------------------- : Simple instructions : quaternary

void instrQuaternary(QuaternaryInstructionType i, ScriptValueP& a, const ScriptValueP& b, const ScriptValueP& c, const ScriptValueP& d) {
	switch (i) {
		case I_RGBA:
			a = to_script(AColor((int)*a, (int)*b, (int)*c, (int)*d));
			break;
	}
}

// ----------------------------------------------------------------------------- : Simple instructions : objects and closures

void Context::makeObject(size_t n) {
	intrusive_ptr<ScriptCustomCollection> ret(new ScriptCustomCollection());
	size_t begin = stack.size() - 2 * n;
	for (size_t i = 0 ; i < n ; ++i) {
		const ScriptValueP& key = stack[begin + 2 * i];
		const ScriptValueP& val = stack[begin + 2 * i + 1];
		ret->value.push_back(val);
		if (key != script_nil) { // valid key
			ret->key_value[key->toString()] = val;
		}
	}
	stack.resize(begin);
	stack.push_back(ret);
}

void Context::makeClosure(size_t n, const Instruction*& instr) {
	intrusive_ptr<ScriptClosure> closure(new ScriptClosure(stack[stack.size() - n - 1]));
	for (size_t j = 0 ; j < n ; ++j) {
		closure->addBinding((Variable)instr[n - j - 1].data, stack.back());
		stack.pop_back();
	}
	// skip arguments
	instr += n;
	// set value, try to simplify
	stack.back() = closure->simplify();
	if (!stack.back()) {
		stack.back() = closure;
	}
}
