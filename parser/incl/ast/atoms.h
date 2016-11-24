#pragma once

#include "ast.h"
#include <optional>

namespace spero::compiler::ast {
	/*
	 * Base class for all sequence nodes
	 *
	 * Extends: ValExpr
	 *   Reuses `prettyPrint` definition
	 *
	 * Exports:
	 *   exprs - a collection of expressions that constitutes the sequence
	 */
	template<class T>
	struct Sequence : ValExpr {
		std::deque<ptr<T>> exprs;

		Sequence(std::deque<ptr<Ast>>&);
	};

	
	/*
	 * Represents a tuple of values
	 *
	 * Extends: Sequence
	 *
	 * Exports: NA
	 */
	struct Tuple : Sequence<ValExpr> {
		Tuple(std::deque<ptr<ValExpr>>&);
		virtual IoStream prettyPrint(IoStream, size_t = 0);
	};


	/*
	 * Represents an array of values
	 *
	 * Extends: Sequence
	 *
	 * Exports: NA
	 */
	struct Array : Sequence<ValExpr> {
		Array(std::deque<ptr<ValExpr>>&);
		virtual IoStream prettyPrint(IoStream, size_t = 0);
	};


	/*
	 * Represents a block of expressions
	 *
	 * Extends: Sequence
	 *
	 * Exports: NA
	 */
	struct Block : Sequence<Ast> {
		Block(std::deque<ptr<Ast>>&);
		virtual IoStream prettyPrint(IoStream, size_t = 0);
	};


	/*
	 * Represents an anonymous type extension
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   cons - the primary constructor for the extended type
	 *   body - the type's body
	 */
	struct TypeExtension : Ast {
		std::optional<ptr<Tuple>> cons;
		ptr<Block> body;

		TypeExtension(ptr<Tuple>, ptr<Block>);
		virtual IoStream prettyPrint(IoStream, size_t = 0);
	};


	/*
	 * Represents a complete step of fuction sequencing
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   caller - expression that applies the function sequence
	 *   anon   - anonymous type extension section
	 *   args   - argument tuple section
	 *   inst   - generic instantiation section
	 */
	struct FnCall : ValExpr {
		ptr<Ast> caller;
		std::optional<ptr<TypeExtension>> anon;
		std::optional<ptr<Tuple>> args;
		std::optional<ptr<Array>> inst;

		FnCall(ptr<Ast>, ptr<TypeExtension>, ptr<Tuple>, ptr<Array>);
		virtual IoStream prettyPrint(IoStream, size_t = 0);
	};


	/*
	 * Represents a range object construction
	 * 
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   start - range beginning value
	 *   stop  - range end value
	 *   step  - range step value [deprecated]
	 */
	struct Range : ValExpr {
		ptr<ValExpr> start, stop;
		std::optional<ptr<ValExpr>> step;

		Range(ptr<ValExpr>, ptr<ValExpr>);
		virtual IoStream prettyPrint(IoStream, size_t = 0);
	};
}