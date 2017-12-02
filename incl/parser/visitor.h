#pragma once

#include "ast.h"

namespace spero::compiler::ast {

	#define DEF_VISIT(typ) inline virtual void visit##typ(typ##& t) { visit(t); }

	/*
	 * Visitor class definition
	 *
	 * Exports:
	 *   accept* - accept method that is called within the visit function to invoke
	 *   accept - method that is called if an overload for the AST type is not provided
	 *       in the child type. This method must be provided to instantiate the type
	 */
	struct Visitor {
		inline virtual void visit(Ast& t) = 0;

		// Base Nodes
		DEF_VISIT(Token);
		DEF_VISIT(Type);
		DEF_VISIT(Statement);
		DEF_VISIT(ValExpr);

		// Literals
		DEF_VISIT(Literal);
		DEF_VISIT(Bool);
		DEF_VISIT(Byte);
		DEF_VISIT(Float);
		DEF_VISIT(Int);
		DEF_VISIT(Char);
		DEF_VISIT(String);
		DEF_VISIT(Future);

		// Atoms
		DEF_VISIT(Tuple);
		DEF_VISIT(Array);
		DEF_VISIT(Block);
		DEF_VISIT(Function);

		// Names
		DEF_VISIT(BasicBinding);
		DEF_VISIT(PathPart);
		DEF_VISIT(Path);
		DEF_VISIT(Pattern);
		DEF_VISIT(TuplePattern);
		DEF_VISIT(VarPattern);
		DEF_VISIT(AdtPattern);
		DEF_VISIT(ValPattern);
		DEF_VISIT(AssignPattern);
		DEF_VISIT(AssignName);
		DEF_VISIT(AssignTuple);

		// Types
		DEF_VISIT(SourceType);
		DEF_VISIT(GenericType);
		DEF_VISIT(TupleType);
		DEF_VISIT(FunctionType);
		DEF_VISIT(AndType);
		DEF_VISIT(OrType);

		// Decorations
		DEF_VISIT(Annotation);
		DEF_VISIT(LocalAnnotation);
		DEF_VISIT(GenericPart);
		DEF_VISIT(TypeGeneric);
		DEF_VISIT(ValueGeneric);
		DEF_VISIT(LitGeneric);
		DEF_VISIT(GenericArray);
		DEF_VISIT(Constructor);
		DEF_VISIT(Adt);
		DEF_VISIT(Argument);
		DEF_VISIT(ArgTuple);

		// Control
		DEF_VISIT(Branch);
		DEF_VISIT(Loop);
		DEF_VISIT(While);
		DEF_VISIT(For);
		DEF_VISIT(IfBranch);
		DEF_VISIT(IfElse);
		DEF_VISIT(Case);
		DEF_VISIT(Match);
		DEF_VISIT(Jump);

		// Statements
		DEF_VISIT(ModDec);
		DEF_VISIT(ImplExpr);
		DEF_VISIT(ModRebindImport);
		DEF_VISIT(SingleImport);
		DEF_VISIT(MultipleImport);
		DEF_VISIT(Rebind);
		DEF_VISIT(Interface);
		DEF_VISIT(TypeAssign);
		DEF_VISIT(VarAssign);
		DEF_VISIT(TypeExtension);

		// Expression
		DEF_VISIT(InAssign);
		DEF_VISIT(Variable);
		DEF_VISIT(UnOpCall);
		DEF_VISIT(BinOpCall);
		DEF_VISIT(Index);
		DEF_VISIT(FnCall);

		// Errors
		//DEF_VISIT(Error);
	};

#undef DEF_VISIT

}