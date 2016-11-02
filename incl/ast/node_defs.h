#pragma once

#include <variant>
#include <memory>
#include "enum.h"

namespace spero::compiler::ast {
	// Organization
	struct Expr;
	struct Literal;

	// Literals
	struct Bool;
	struct Byte;
	struct Float;
	struct Int;
	struct String;
	struct Char;
	struct FnBody;

	// Bindings
	struct BasicBinding;
	struct QualBinding;
	struct Type;

	// Atoms
	struct AnonType;
	struct Sequence;
	struct FnCall;

	// Language Decorators
	struct Annotation;
	struct TypeTuple;
	struct FullType;
	struct TypeGeneric;
	struct ValueGeneric;
	struct GenArray;
	struct InstArray;
	struct Case;
	struct ImportPathPart;

	// Assignment
	struct Adt;
	struct AssignTuple;
	struct AssignPattern;
	struct ImportRebind;
	struct PatternTuple;
	struct NamedPattern;
	struct TuplePattern;
	struct AdtPattern;
	struct Pattern;
	struct AssignCore;
	struct VarAssign;
	struct Interface;
	struct TypeAssign;

	// Dot Control
	struct IfCore;
	struct WhileCore;
	struct ForCore;
	struct LoopCore;
	struct JmpCore;
	struct MatchCore;

	// Control
	struct IfBranch;
	struct BranchStmt;
	struct WhileLoop;
	struct ForLoop;
	struct Loop;
	struct Jump;
	struct Match;

	// Statements
	struct Index;
	struct Binary;
	struct Range;
	struct Infix;
	struct ImplExpr;
	struct ModDec;
	struct ModImport;

	// Predefs
	struct Any {};
	struct Sentinel {};
	BETTER_ENUM(KeywordType, char, LET, DEF, STATIC, MUT, DO,
		MOD, USE, MATCH, IF, ELSIF, ELSE, WHILE, FOR, LOOP,
		BREAK, CONT, YIELD, RET, WAIT, IMPL, F_IN);
	BETTER_ENUM(PtrStyling, char, POINTER, VIEW, DEREF, NONE);
	BETTER_ENUM(VarianceType, char, COVARIANT, CONTRAVARIANT, INVARIANT, NA);
	BETTER_ENUM(RelationType, char, IMPLS, NOT_IMPLS, SUBTYPE, SUPERTYPE);
	BETTER_ENUM(VisibilityType, char, PUBLIC, PROTECTED, PRIVATE, STATIC);
	BETTER_ENUM(BindingType, char, TYPE, VARIABLE, OPERATOR);
	BETTER_ENUM(JumpType, char, YIELD, BREAK, WAIT, CONTINUE, RETURN);
	BETTER_ENUM(SequenceType, char, SCOPE, TUPLE, ARRAY);
}

namespace spero::compiler {
	// Representing non-evaluable nodes that represent bindings
	using binding = std::variant<
		ast::BasicBinding, ast::QualBinding, ast::Type
	>;

	// Representing non-evaluable nodes that decorate other expressions
	using decorators = std::variant<
		ast::Annotation, ast::FullType, ast::TypeGeneric, ast::ValueGeneric,
		ast::TypeTuple, ast::GenArray, ast::Case, ast::ImportPathPart, ast::InstArray
	>;

	// Representing non-evaluable nodes that build up assignments
	using assignment = std::variant<
		ast::Adt, ast::AssignTuple, ast::AssignPattern, ast::AnonType,
		ast::ImportRebind, ast::PatternTuple, ast::Pattern, ast::Any,
		ast::NamedPattern, ast::TuplePattern, ast::AdtPattern
	>;

	// Representing contstruction pieces
	using con_pieces = std::variant<
		ast::KeywordType, ast::PtrStyling, ast::VarianceType,
		ast::RelationType, ast::VisibilityType, ast::Sentinel,
		ast::BindingType, ast::JumpType, ast::SequenceType
	>;

	// Representing evaluable nodes
	using astnode = std::unique_ptr<ast::Expr>;
		// Literal    = Bool, Byte, Float, Int, String, Char, FnBody
		// AssignCore = VarAssign, TypeAssign, Interface
		// Atoms      = Sequence, FnCall
		// Dot-Cntrl  = IfCore, WhileCore, ForCore, LoopCore, JmpCore, MatchCore
		// Control    = IfBranch, BranchStmt, WhileLoop, ForLoop, Loop, Jump, Match
		// Stmts      = Index, Range, Binary, ImplExpr, ModDec, ModImport

	// Values that can be pushed on the stack 
	using StackVals = std::variant<astnode, binding, decorators, assignment, con_pieces>;
}
