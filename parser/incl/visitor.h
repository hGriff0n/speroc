#pragma once

#include "ast/ast.h"

namespace spero::compiler::ast {

	#define DEF_ACCEPT(typ) inline virtual void accept##typ(typ##& t) { accept(t); }

	/*
	 * Visitor class definition
	 *
	 * Exports:
	 *   accept* - accept method that is called within the visit function to invoke 
	 *   accept - method that is called if an overload for the AST type is not provided
	 *       in the child type. This method must be provided to instantiate the type
	 */
	struct Visitor {
		inline virtual void accept(Ast& t) =0;

		// Base Nodes
		DEF_ACCEPT(Token)
		DEF_ACCEPT(Type)
		DEF_ACCEPT(Stmt)
		DEF_ACCEPT(ValExpr)

		// Literals
		DEF_ACCEPT(Bool)
		DEF_ACCEPT(Byte)
		DEF_ACCEPT(Float)
		DEF_ACCEPT(Int)
		DEF_ACCEPT(Char)
		DEF_ACCEPT(String)
		DEF_ACCEPT(Tuple)
		DEF_ACCEPT(Array)
		DEF_ACCEPT(Block)

		// Names
		DEF_ACCEPT(BasicBinding)
		DEF_ACCEPT(QualifiedBinding)
		DEF_ACCEPT(Variable)
		DEF_ACCEPT(AssignPattern)
		DEF_ACCEPT(AssignName)
		DEF_ACCEPT(AssignTuple)
		DEF_ACCEPT(Pattern)
		DEF_ACCEPT(PTuple)
		DEF_ACCEPT(PNamed)
		DEF_ACCEPT(PAdt)
		DEF_ACCEPT(PVal)

		// Types
		DEF_ACCEPT(SourceType)
		DEF_ACCEPT(GenericType)
		DEF_ACCEPT(TupleType)
		DEF_ACCEPT(FuncType)

		// Decorations
		DEF_ACCEPT(Annotation)
		DEF_ACCEPT(LocalAnnotation)
		DEF_ACCEPT(GenericPart)
		DEF_ACCEPT(TypeGeneric)
		DEF_ACCEPT(ValueGeneric)
		DEF_ACCEPT(TypeExt)
		DEF_ACCEPT(Case)
		DEF_ACCEPT(ImportPiece)
		DEF_ACCEPT(ImportName)
		DEF_ACCEPT(ImportGroup)
		DEF_ACCEPT(Adt)
		DEF_ACCEPT(Future)

		// Control
		DEF_ACCEPT(Branch)
		DEF_ACCEPT(Loop)
		DEF_ACCEPT(While)
		DEF_ACCEPT(For)
		DEF_ACCEPT(IfElse)
		DEF_ACCEPT(Match)
		DEF_ACCEPT(JumpExpr)
		DEF_ACCEPT(Wait)
		DEF_ACCEPT(Break)
		DEF_ACCEPT(Continue)
		DEF_ACCEPT(Return)
		DEF_ACCEPT(YieldRet)

		// Stmts
		DEF_ACCEPT(FnBody)
		DEF_ACCEPT(FnCall)
		DEF_ACCEPT(Range)
		DEF_ACCEPT(UnaryOpApp)
		DEF_ACCEPT(Interface)
		DEF_ACCEPT(TypeAssign)
		DEF_ACCEPT(VarAssign)
		DEF_ACCEPT(InAssign)
		DEF_ACCEPT(ImplExpr)
		DEF_ACCEPT(ModDec)
		DEF_ACCEPT(ModImport)
		DEF_ACCEPT(Index)
	};

	#undef DEF_ACCEPT
}

namespace spero::compiler::ast {
	/*
	* Definitions for any templated classes
	*/
	template<class T, class Inher>
	Visitor& Sequence<T, Inher>::visit(Visitor& v) {
		v.accept(*this);
		return v;
	}
}