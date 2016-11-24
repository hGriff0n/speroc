#pragma once

#include "ast.h"

namespace spero::compiler::ast {
	/*
	 * Represents a known boolean constant
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *  val - value of the constant
	 */
	struct Bool : ValExpr {
		bool val;
		Bool(bool);

		virtual IoStream prettyPrint(IoStream, size_t = 0);
	};


	/*
	 * Represents a known byte constant
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *  val - value of the constant
	 */
	struct Byte : ValExpr {
		unsigned long val;
		Byte(std::string, int);

		virtual IoStream prettyPrint(IoStream, size_t = 0);
	};


	/*
	 * Represents a known floating point constant
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *  val - value of the constant
	 */
	struct Float : ValExpr {
		double val;
		Float(std::string);

		virtual IoStream prettyPrint(IoStream, size_t = 0);
	};


	/*
	 * Represents a known integer constant
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *  val - value of the constant
	 */
	struct Int : ValExpr {
		long val;
		Byte(std::string);

		virtual IoStream prettyPrint(IoStream, size_t = 0);
	};


	/*
	 * Represents a known string constant
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *  val - value of the constant
	 */
	struct String : ValExpr {
		std::string val;
		String(std::string);

		virtual IoStream prettyPrint(IoStream, size_t = 0);
	};


	/*
	 * Represents a known character constant
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *  val - value of the constant
	 */
	struct Char : ValExpr {
		char val;
		Char(char);

		virtual IoStream prettyPrint(IoStream, size_t = 0);
	};


	/*
	 * Represents a function lambda
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   forward - flag for whether the function body forwards arguments to the first expression
	 *   args    - tuple for function binding function arguments
	 *   ret     - return type of the function (Note: type = args.type -> ret)
	 *   body    - expression that the function evaluates
	 */
	struct FnBody : ValExpr {
		bool forward;
		ptr<Tuple> args;
		ptr<Type> ret;
		ptr<ValExpr> body;

		FnBody(ptr<ValExpr>, bool);
		virtual IoStream prettyPrint(IoStream, size_t = 0);
	};
}