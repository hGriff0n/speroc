#pragma once

#include <variant>

namespace spero::compiler::ast {
	// Literals
	struct Byte;
	struct Int;
	struct Float;
	struct String;
	struct Char;
	struct Bool;
	struct Tuple;
	struct Array;
	struct FnObj;

	// Bindings
	struct BasicName;
	struct Operator;
	struct NamePath;
	struct QualName;
	struct Type;
	struct Pattern;		// I'm uncertain I'm going to implement Patterns atm

	// Atoms
	struct FnCall;
	struct Scope;
	struct Case;
	struct Match;

	// Decorators
	struct Annotation;
	struct ModDecl;
	struct TypeTuple;
	struct TypeInfer;
	struct GenSubtype;
	struct GenType;
	struct GenValue;
	struct GenArray;

	// Assignment
	struct ADT;
	struct VarAssign;
	struct TypeAssign;
	struct Assignment;

	// Expressions
	struct Index;
	struct InfixOp;
	struct IfStmt;
	struct Branch;			// Construction Node
	struct UsePath;			// Construction Node
	struct UsePathElem;		// Construction Node
	struct UseImport;		// Construction Node
	struct UseStmt;

	// Ast Productions
	struct Sentinel {};
	enum class Keyword {
		MUT,
		LET,
		DEF,
		STATIC,
		USE,
		MOD,
		MATCH,
		IF,
		ELSIF,
		ELSE,
	};
	enum class PtrStyling {
		POINTER,
		VIEW,
		NONE,
	};
	enum class Variance {
		COVARIANT,
		CONTRAVARIANT,
		NONE,
	};
	enum class SubtypeRelation {
		IMPLEMENTS,
		NOT_IMPLEMENTS,
		SUBTYPE,
		SUPERTYPE,
	};
}

namespace spero::compiler {
	using litnode = std::variant<ast::Byte, ast::Int, ast::Float, ast::FnObj,
		ast::String, ast::Char, ast::Bool, ast::Tuple, ast::Array>;

	using bindnode = std::variant<ast::BasicName, ast::Operator, ast::Pattern, 
		ast::QualName, ast::Type>;

	using atomnode = std::variant<ast::FnCall, ast::Scope, ast::Match>;

	using decnode = std::variant<ast::Annotation, ast::ModDecl, ast::TypeTuple,
		ast::TypeInfer, ast::GenType, ast::GenValue, ast::GenArray, ast::SubtypeRelation>;

	using connode = std::variant<ast::Sentinel, ast::Keyword, ast::PtrStyling, ast::Variance,
		bool, ast::Branch, ast::UsePathElem, ast::UseImport, ast::NamePath, ast::Case>;
	
	using assignnode = std::variant<ast::ADT, ast::VarAssign, ast::TypeAssign, ast::Assignment>;

	using exprnode = std::variant<ast::Index, ast::InfixOp, ast::IfStmt, ast::UseStmt>;

	using astnode = std::variant<litnode, bindnode, atomnode, decnode, connode, assignnode, exprnode>;
}
