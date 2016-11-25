#pragma once

#include "ast.h"
#include <optional>

namespace spero::compiler::ast {
	/*
	 * Forward Declaration
	 */
	struct AssignPattern;
	struct QualifiedBinding;
	struct Block;
	struct GenericPart;
	struct ImportPiece;

	using GenArray = std::deque<ptr<GenericPart>>;


	/*
	 * Base case for handling assignment statements
	 *   Instance also handles interface specification
	 *
	 * Extends: Stmt
	 *
	 * Exports:
	 *   vis - visibility of the created binding
	 *   name - pattern that represents the binding
	 *   generics - collection of generic parts for the assignment
	 *   type - ???
	 */
	struct Interface : Stmt {
		VisibilityType vis;
		ptr<AssignPattern> name;
		GenArray generics;
		std::optional<ptr<Type>> type;

		Interface(ptr<AssignPattern>, GenArray, ptr<Type>);
		virtual OutStream& prettyPrint(OutStream&, size_t = 0, std::string = "");
	};

	
	/*
	 * Instance class for specifically handling type binding
	 *
	 * Extends: Interface
	 *   Uses `type` for inheritance
	 *
	 * Exports:
	 *   cons - list of adt and primary constructors
	 *   body - type body
	 */
	struct TypeAssign : Interface {
		std::deque<ptr<Ast>> cons;				// must be Adt or Tuple, only one Tuple allowed
		ptr<Block> body;

		TypeAssign(ptr<AssignPattern>, std::deque<ptr<Ast>>, GenArray, ptr<Block>, ptr<Type>);
		virtual OutStream& prettyPrint(OutStream&, size_t = 0, std::string = "") final;
	};

	
	/*
	 * Instance class for handling value assignment to variables and operators
	 *
	 * Extends: Interface
	 *   Uses `type` for static checking
	 *
	 * Exports:
	 *   expr - value to be assigned to the binding
	 */
	struct VarAssign : Interface {
		ptr<ValExpr> expr;						// must be a FnBody if name is a Operator

		VarAssign(ptr<AssignPattern>, GenArray, ptr<ValExpr>, ptr<Type>);
		virtual OutStream& prettyPrint(OutStream&, size_t = 0, std::string = "") final;
	};


	/*
	 * Represents an implements expression within a type body
	 *
	 * Extends: Stmt
	 *
	 * Exports:
	 *   type - the type that is being implemented
	 */
	struct ImplExpr : Stmt {
		ptr<QualifiedBinding> type;

		ImplExpr(ptr<QualifiedBinding>);
		virtual OutStream& prettyPrint(OutStream&, size_t = 0, std::string = "") final;
	};


	/*
	 * Represents a module declaration expression
	 *
	 * Extends: Stmt
	 *
	 * Exports:
	 *   module - the name of the created module
	 */
	struct ModDec : Stmt {
		ptr<QualifiedBinding> module;

		ModDec(ptr<QualifiedBinding>);
		virtual OutStream& prettyPrint(OutStream&, size_t = 0, std::string = "") final;
	};


	/*
	 * Represents a module import expression
	 *
	 * Extends: Stmt
	 *
	 * Exports:
	 *   parts - the module path of the imported bindings
	 */
	struct ModImport : Stmt {
		std::deque<ptr<ImportPiece>> parts;

		ModImport(std::deque<ptr<ImportPiece>>);
		virtual OutStream& prettyPrint(OutStream&, size_t = 0, std::string = "") final;
	};


	/*
	 * Represents a non-disturbed sequence of indexes
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   elems - the sequence of index values (indexing goes ->)
	 */
	struct Index : ValExpr {
		std::deque<ptr<ValExpr>> elems;

		Index(ptr<ValExpr>, ptr<ValExpr>);
		virtual OutStream& prettyPrint(OutStream&, size_t = 0, std::string = "") final;
	};
}