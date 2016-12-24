#pragma once

#include "ast.h"

namespace spero::compiler::ast {
	/*
	 * Forward Declarations
	 */
	struct Pattern;
	struct Case;


	/*
	 * Represents a sequence of branching constructs
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   if_branches - sequence of conditional branching
	 *   else_branch - optional fall-through case
	 */
	struct Branch : ValExpr {
		using Pair = std::pair<ptr<ValExpr>, ptr<ValExpr>>;

		std::deque<Pair> if_branches;
		ptr<ValExpr> else_branch;

		Branch(ptr<ValExpr>, ptr<ValExpr>, Ast::Location);
		void addBranch(ptr<ValExpr>, ptr<ValExpr>);
		void addBranch(ptr<ValExpr>);

		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};

	/*
	 * An infinite loop that just repeats the body
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   body - the body of the loop
	 */
	struct Loop : ValExpr {
		ptr<ValExpr> body;

		Loop(ptr<ValExpr>, Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Represents a basic while loop construct
	 *
	 * Extends: ValExpr
	 * 
	 * Exports:
	 *   test - the expression to test for loop termination
	 *   body - the body of the loop
	 */
	struct While : ValExpr {
		ptr<ValExpr> test, body;

		While(ptr<ValExpr>, ptr<ValExpr>, Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Represents a for loop construct
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   pattern - the var pattern to hold iteration values
	 *   generator - the sequence that generates iteration values
	 *   body - the body of the loop
	 */
	struct For : ValExpr {
		ptr<Pattern> pattern;
		ptr<ValExpr> generator, body;

		For(ptr<Pattern>, ptr<ValExpr>, ptr<ValExpr>, Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Represents a match construct
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   switch_expr - value to match against
	 *   cases - sequence of case statements to use in matching
	 */
	struct Match : ValExpr {
		ptr<ValExpr> switch_expr;
		std::deque<ptr<Case>> cases;

		Match(ptr<ValExpr>, std::deque<ptr<Case>>, Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Base case for "jump" expressions
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   expr - evaluable body
	 *   jmp - keyword to distinguish jump bodies [deprecated]
	 */
	struct Jump : ValExpr {
		ptr<ValExpr> expr;
		KeywordType jmp;

		Jump(KeywordType, ptr<ValExpr>, Ast::Location);
	};

	/*
	* Instance class to represent a wait expression
	*
	* Extends: Jump
	*/
	struct Wait : Jump {
		Wait(ptr<ValExpr>, Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Instance class to represent a break expression
	 *
	 * Extends: Jump
	 */
	struct Break : Jump {
		Break(ptr<ValExpr>, Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Instance class to represent a continue expression
	 *
	 * Extends: Jump
	 */
	struct Continue : Jump {
		Continue(ptr<ValExpr>, Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Instance class to represent a return expression
	 *
	 * Extends: Jump
	 */
	struct Return : Jump {
		Return(ptr<ValExpr>, Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Instance class to represent a yield expression
	 *
	 * Extends: Jump
	 */
	struct YieldRet : Jump {
		YieldRet(ptr<ValExpr>, Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};
}