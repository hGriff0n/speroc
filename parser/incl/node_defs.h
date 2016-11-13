#pragma once

#include <variant>
#include <memory>
#include <vector>
#include "enum.h"

namespace spero::compiler::ast {
	// Organization
	struct Ast;
	struct Token;
	struct Stmt;
	struct ValExpr;

	// Literals
	struct Bool;				//
	struct Byte;				//
	struct Float;				//
	struct Int;					//
	struct String;				//
	struct Char;				//

	// Bindings
	struct BasicBinding;		//
	struct QualBinding;			//

	// Types
	struct Type;
	struct BasicType;
	struct GenType;
	struct InstType;
	struct TupleType;
	struct FuncType;

	// Atoms
	struct TypeExt;
	struct Sequence;
	struct Array;
	struct Tuple;
	struct Block;
	struct FnCall;
	struct FnBody;
	struct Range;

	// Language Decorators
	struct Annotation;
	struct GenericPart;
	struct TypeGeneric;
	struct ValueGeneric;
	struct Case;
	struct ImportPart;
	struct ImportPiece;			// Base class represents any
	struct ImportName;

	// Assignment
	struct Adt;
	struct AssignPattern;		// Base class represents any
	struct AssignName;
	struct AssignTuple;
	struct Pattern;				// Base class represents any
	struct PTuple;
	struct PNamed;
	struct PAdt;
	struct Interface;
	struct VarAssign;
	struct TypeAssign;

	// Control
	struct Branch;
	struct While;
	struct For;
	struct Loop;				//
	struct Jump;
	struct Wait;				//
	struct Break;
	struct Continue;
	struct Return;
	struct Yield;
	struct Match;

	// Statements
	struct Index;
	struct Infix;
	struct ImplExpr;
	struct ModDec;
	struct ModImport;

	// Predefs
	using Sentinel = nullptr_t;
	BETTER_ENUM(KeywordType, char, LET, DEF, STATIC, MUT, DO,
		MOD, USE, MATCH, IF, ELSIF, ELSE, WHILE, FOR, LOOP,
		BREAK, CONT, YIELD, RET, WAIT, IMPL, F_IN);
	BETTER_ENUM(PtrStyling, char, POINTER, VIEW, NA);
	BETTER_ENUM(VarianceType, char, COVARIANT, CONTRAVARIANT, INVARIANT, NA);
	BETTER_ENUM(RelationType, char, IMPLS, NOT_IMPLS, SUBTYPE, SUPERTYPE, NA);
	BETTER_ENUM(VisibilityType, char, PUBLIC, PROTECTED, PRIVATE, STATIC);
	BETTER_ENUM(BindingType, char, TYPE, VARIABLE, OPERATOR);
	BETTER_ENUM(UnaryType, char, DEREF, NOT, MINUS, NA);
}

namespace spero::compiler {
	template<class T>
	using ptr = std::unique_ptr<T>;

	// Representing non-evaluable nodes
	using node = ptr<ast::Ast>;

	// Representing evaluable nodes
	using value = ptr<ast::ValExpr>;
		// Literal    = Bool, Byte, Float, Int, String, Char, FnBody
		// AssignCore = VarAssign, TypeAssign, Interface
		// Atoms      = Sequence, FnCall
		// Dot-Cntrl  = IfCore, WhileCore, ForCore, LoopCore, JmpCore, MatchCore
		// Control    = IfBranch, BranchStmt, WhileLoop, ForLoop, Loop, Jump, Match
		// Stmts      = Index, Range, Binary

	using GenArray = std::vector<ptr<ast::GenericPart>>;
}
