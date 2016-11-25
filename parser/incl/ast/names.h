#pragma once

#include "ast.h"

namespace spero::compiler::ast {
	/*
	 * Represents the basic block of a Spero binding
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   name - the string representation of the binding
	 *   type - is the string a variable, type, or operator
	 *
	 * TODO:
	 *   Split this class into Operator/Variable/Type binding classes ???
	 */
	struct BasicBinding : Ast {
		std::string name;
		BindingType type;

		BasicBinding(std::string, BindingType);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Represents a complete Spero binding
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   parts - a collection of BasicBindings that constructs the QualBinding
	 */
	struct QualifiedBinding : Ast {
		std::deque<ptr<BasicBinding>> parts;

		QualifiedBinding(ptr<BasicBinding>);
		QualifiedBinding(std::deque<ptr<BasicBinding>>);

		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Basic class that represents a variable name
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   name - qualified binding that represents the variable
	 */
	struct Variable : ValExpr {
		ptr<QualifiedBinding> name;

		Variable(ptr<QualifiedBinding>);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Base class for representing an assignable binding
	 *   Instance doubles for representing the any case
	 *
	 * Extends: Ast
	 *
	 * Note: Instance usage is currently deprecated
	 */
	struct AssignPattern : Ast {
	};


	/*
	 * Represents a singular named binding in an assignment context
	 *
	 * Extends: AssignPattern
	 *
	 * Exports:
	 *   name - binding to assign to
	 */
	struct AssignName : AssignPattern {
		ptr<BasicBinding> var;

		AssignName(ptr<BasicBinding>);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Represents a decomposed tuple assignment
	 *
	 * Extends: AssignPattern
	 *
	 * Exports:
	 *   vars - a collection of AssignPattern to match against
	 */
	struct AssignTuple : AssignPattern {
		std::deque<ptr<AssignPattern>> vars;

		AssignTuple(std::deque<ptr<AssignPattern>>);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Base class for representing pattern matching expressions
	 *   Instance doubles for representing the any case
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   is_mut - flag for whether bound value should be mutable
	 */
	struct Pattern : Ast {
		bool is_mut = false;
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "");
	};


	/*
	 * Instance class for matching against a decomposed tuple
	 *
	 * Extends: Pattern
	 *
	 * Exports:
	 *   ptns - collection of sub-patterns to match
	 */
	struct PTuple : Pattern {
		std::deque<ptr<Pattern>> ptns;

		PTuple(std::deque<ptr<Pattern>>);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};

	
	/*
	 * Instance class for binding a value in a pattern match
	 *
	 * Extends: Pattern
	 *
	 * Exports:
	 *   name - the value to bind to
	 */
	struct PNamed : Pattern {
		ptr<BasicBinding> name;

		PNamed(ptr<BasicBinding>);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "");
	};


	/*
	 * Instance class for matching against a decomposed ADT
	 *
	 * Extends: PNamed
	 *   Uses name as the ADT type to match againts
	 *
	 * Exports:
	 *   args - the tuple to match values against
	 */
	struct PAdt : PNamed {
		ptr<PTuple> args;
		// Note: assert(name.type == BindingType::TYPE);

		PAdt(ptr<BasicBinding>, ptr<PTuple>);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Instance class for matching against a value
	 *   Note: [deprecated]
	 *
	 * Extends: Pattern
	 *
	 * Exports:
	 *   value - value to match against
	 */
	struct PVal : Pattern {
		ptr<ValExpr> value;

		PVal(ptr<ValExpr>);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};
}