#pragma once

#include "ast.h"

namespace spero::compiler::ast {
	/*
	 * Forward Declarations
	 */
	struct Tuple;


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
		Bool(bool, Ast::Location);

		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
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
		Byte(std::string, int, Ast::Location);

		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
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
		Float(std::string, Ast::Location);

		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
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
		Int(std::string, Ast::Location);

		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
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
		String(std::string, Ast::Location);

		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
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
		Char(char, Ast::Location);

		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "");
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

		FnBody(ptr<ValExpr>, bool, Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "");
	};
}