#pragma once

#include "ast/node_defs.h"

#include <optional>
#include <string>
#include <vector>
#include <memory>		// using std::shared_ptr as a cycle "breaker"

#define PRTY_OUT std::cout

namespace spero::compiler::ast {
	// Forward Defining
	struct GenArray {
		std::vector<std::variant<TypeGeneric, ValueGeneric>> elems;
	};
	struct InstArray {
		std::vector<std::variant<Type, astnode>> elems;
	};

	//
	// Bindings
	//
	struct BasicBinding {
		std::string name;
		BindingType type;

		BasicBinding(const std::string&, BindingType);
	};
	struct QualBinding {
		std::vector<BasicBinding> val;
	};
	struct Type {
		QualBinding name;
		GenArray generics;
		PtrStyling ptr;
		bool is_mut;
	};
	struct TypeTuple {
		std::vector<FullType> types;
	};
	struct FullType {
		TypeTuple args;
		Type val;
	};


	//
	// Organization
	//
	struct Expr {
		bool is_mut = false;
		bool deref = false;
		std::vector<Annotation> annots;
		std::optional<FullType> type;

		virtual std::string pretty_fmt(int = 0);
	};


	//
	// Literals
	//
	struct Literal : Expr {};
	struct Bool : Literal {
		bool val;
		Bool(bool);

		std::string pretty_fmt(int = 0);
	};
	struct Byte : Literal {
		unsigned long val;
		Byte(const std::string&, int);

		std::string pretty_fmt(int = 0);
	};
	struct Float : Literal {
		double val;
		Float(const std::string&);

		std::string pretty_fmt(int = 0);
	};
	struct Int : Literal {
		int val;
		Int(const std::string&);

		std::string pretty_fmt(int = 0);
	};
	struct String : Literal {
		std::string val;
		String(const std::string&);

		std::string pretty_fmt(int = 0);
	};
	struct Char : Literal {
		char val;
		Char(char);

		std::string pretty_fmt(int = 0);
	};
	// This needs work
	struct FnArgs {
		using NamedArg = std::pair<BasicBinding, std::optional<FullType>>;
		std::vector<std::variant<FullType, NamedArg>> val;
	};
	struct FnBody : Expr {			// type = return type
		FnArgs args;
		bool is_fwd;
		astnode body;
	};


	//
	// Atoms
	//
	struct AnonType {
		std::optional<FnArgs> cons;
		astnode body;
	};
	struct Sequence : Expr {
		std::vector<astnode> vals;
		SequenceType sequ;
	};
	struct FnCall : Expr {
		QualBinding fn;
		InstArray generics;
		std::vector<std::variant<std::unique_ptr<Sequence>, AnonType>> chain;			// Sequence must be a tuple
	};


	//
	// Language Decorators
	//
	struct Annotation {
		std::string name;
		bool global;
		std::unique_ptr<Sequence> args;													// Sequence must be a tuple
	};
	struct TypeGeneric {
		BasicBinding name;
		VarianceType variant;
		RelationType relation;
		FullType reltype;
	};
	struct ValueGeneric {
		BasicBinding name;
		FullType type;
		astnode val;						// Note: optional, null = not given
	};
	struct Case {
		std::vector<Pattern> vars;
		astnode val;
	};
	struct ImportPathPart {
		std::variant<Any, std::vector<std::variant<BasicBinding, ImportRebind>>> val;
	};


	//
	// Assignment
	//
	struct ImportRebind {
		BasicBinding old_name, new_name;
	};
	struct PatternTuple {
		std::vector<Pattern> vals;
	};
	struct Adt {
		BasicBinding name;
		TypeTuple types;
	};
	struct AssignTuple {
		std::vector<AssignPattern> vars;
	};
	struct AssignPattern {
		std::variant<BasicBinding, AssignTuple> val;
	};
	struct NamedPattern {
		bool is_mut;
		BasicBinding var;
	};
	struct TuplePattern {
		bool is_mut;
		PatternTuple vars;
	};
	struct AdtPattern {
		BasicBinding adt;
		std::optional<PatternTuple> vars;
	};
	struct Pattern {
		std::variant<Any, NamedPattern, TuplePattern, PatternTuple, AdtPattern> val;
	};
	struct AssignCore : Expr {
		VisibilityType vis;
		GenArray generics;
		AssignPattern pattern;
	};
	struct VarAssign : AssignCore {
		astnode val;
	};
	struct TypeAssign : VarAssign {
		std::vector<std::variant<Adt, FnArgs>> cons;
	};
	struct Interface : AssignCore {
	};					// type must be a option:some


	//
	// Dot Control
	//
	struct IfCore : Expr {
		astnode body;
	};
	struct ForCore : Expr {
		Pattern vars;
		astnode generator;
	};
	struct JmpCore : Expr {
		JumpType jmp;
	};
	struct MatchCore : Expr {
		std::vector<Case> cases;
	};
	struct LoopCore : Expr {
	};
	struct WhileCore : IfCore {
	};


	//
	// Control
	//
	struct IfBranch : IfCore {
		astnode test;
	};
	struct BranchStmt : Expr {
		std::vector<std::unique_ptr<IfBranch>> if_bs;
		astnode else_b;						// Note: optional, null = not given
	};
	struct WhileLoop : IfBranch {
	};
	struct ForLoop : ForCore, IfCore {
	};
	struct Loop : IfCore {
	};
	struct Jump : JmpCore {
		astnode expr;						// Note: optional, null = not given
	};
	struct Match : MatchCore {
		astnode sw_val;
	};


	// Statements
	struct Index : Expr {
		std::vector<astnode> indices;
	};
	struct Infix : Expr {
		astnode lhs, rhs;
		BasicBinding op;
	};
	struct Range : Expr {
		astnode start, end;
		astnode step;						// Note: optional, null = not given
	};
	struct ImplExpr : Expr {
		QualBinding typ;
	};
	struct ModDec : Expr {
		std::vector<BasicBinding> path;
	};
	struct ModImport : Expr {
		std::vector<ImportPathPart> path;
	};
}


namespace spero::util {
	#define PRETTY_PRINT(type) template<class Stream> \
	Stream& pretty_print(const spero::compiler::type& root, Stream& s, int depth = 0)

	#define PRETTY_PRINT_SCAFF(type) PRETTY_PRINT(type) { return s; }

	// Normal Expressions
	PRETTY_PRINT(astnode) {
		return (Stream&)(s << root->pretty_fmt(depth));
	}

	// Bindings
	PRETTY_PRINT(ast::BasicBinding) {
		s << std::string(depth, ' ') << root.name << "(" << root.type << ")";
		return s;
	}
	PRETTY_PRINT_SCAFF(ast::QualBinding)
	PRETTY_PRINT_SCAFF(ast::Type)

	// Decorators
	PRETTY_PRINT_SCAFF(ast::Annotation)
	PRETTY_PRINT_SCAFF(ast::FullType)
	PRETTY_PRINT_SCAFF(ast::TypeGeneric)
	PRETTY_PRINT_SCAFF(ast::ValueGeneric)
	PRETTY_PRINT_SCAFF(ast::TypeTuple)
	PRETTY_PRINT_SCAFF(ast::GenArray)
	PRETTY_PRINT_SCAFF(ast::Case)
	PRETTY_PRINT_SCAFF(ast::ImportPathPart)
	PRETTY_PRINT_SCAFF(ast::InstArray)

	// Assignment
	PRETTY_PRINT_SCAFF(ast::Adt)
	PRETTY_PRINT_SCAFF(ast::AssignTuple)
	PRETTY_PRINT_SCAFF(ast::AssignPattern)
	PRETTY_PRINT_SCAFF(ast::AnonType)
	PRETTY_PRINT_SCAFF(ast::ImportRebind)
	PRETTY_PRINT_SCAFF(ast::PatternTuple)
	PRETTY_PRINT_SCAFF(ast::Any)
	PRETTY_PRINT_SCAFF(ast::NamedPattern)
	PRETTY_PRINT_SCAFF(ast::TuplePattern)
	PRETTY_PRINT_SCAFF(ast::AdtPattern)

	// Construction Pieces
	PRETTY_PRINT(con_pieces) {
		using namespace spero::compiler::ast;
		s << std::string(depth, ' ');

		std::visit(compose(
			[&s](const KeywordType& k) { s << "Keyword: " << k; },
			[&s](const PtrStyling& p) { s << "Ptr: " << p; },
			[&s](const VarianceType& v) { s << "Variance: " << v; },
			[&s](const RelationType& r) { s << "Relation: " << r; },
			[&s](const VisibilityType& v) { s << "Visibility: " << v; },
			[&s](const Sentinel&) { s << "Sentinel"; },
			[&s](const BindingType& b) { s << "BindType: " << b; },
			[&s](const JumpType& j) { s << "Jump: " << j; },
			[&s](const SequenceType& seq) { s << "SeqType: " << seq; }
		), root);

		return s;
	}

	// Stack Values
	PRETTY_PRINT(StackVals) {
		std::visit(compose(
			[&s, depth](const spero::compiler::astnode& a) { pretty_print(a, s, depth); },
			[&s, depth](const spero::compiler::con_pieces& c) { pretty_print(c, s, depth); },
			[&s, depth](const auto& v) {
				std::visit([&s, depth](const auto& elem) { pretty_print(elem, s, depth); }, v);
			}
		), root);

		return s;
	}

	#undef PRETTY_PRINT_SCAFF
	#undef PRETTY_PRINT
}
