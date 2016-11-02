#pragma once

#include "ast/node_defs.h"

#include <optional>
#include <string>
#include <vector>
#include <memory>		// using std::shared_ptr as a cycle "breaker"


/*
 Current problems:
   FunctionArgument solution is hacky (grammar may be broken too)
 */

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
	};


	//
	// Literals
	//
	struct Literal : Expr {};
	struct Bool : Literal {
		bool val;
		Bool(bool);
	};
	struct Byte : Literal {
		unsigned long val;
		Byte(const std::string&, int);
	};
	struct Float : Literal {
		double val;
		Float(const std::string&);
	};
	struct Int : Literal {
		int val;
		Int(const std::string&);
	};
	struct String : Literal {
		std::string val;
		String(const std::string&);
	};
	struct Char : Literal {
		char val;
		Char(char);
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
	struct Interface : AssignCore {};													// type must be a option:some


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
	struct LoopCore : Expr {};
	struct WhileCore : IfCore {};


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
	struct WhileLoop : IfBranch {};
	struct ForLoop : ForCore, IfCore {};
	struct Loop : IfCore {};
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
	template<class Stream>
	Stream& pretty_print(spero::compiler::astnode& root, Stream& s, int depth = 0) {

	}

	template<class Stream>
	Stream& pretty_print(spero::compiler::binding& b, Stream& s, int depth = 0) {

	}

	template<class Stream>
	Stream& pretty_print(spero::compiler::decorators& d, Stream& s, int depth = 0) {

	}

	template<class Stream>
	Stream& pretty_print(spero::compiler::assignment& a, Stream& s, int depth = 0) {

	}

	template<class Stream>
	Stream& pretty_print(spero::compiler::StackVals& root, Stream& s, int depth = 0) {
		using namespace spero::compiler;
		using namespace compiler::ast;

		s << std::string(depth, ' ');

		std::visit(compose(
			[&s](astnode& a) { s << "An astnode was found\n"; },
			[&s](binding& b) { s << "A binding was found\n"; },
			[&s](decorators& d) { s << "A decorator was found\n"; },
			[&s](assignment& a) { s << "An assignment piece was found\n"; },
			[&s](con_pieces& c) {
				std::visit(compose(
					[&s](ast::KeywordType& k) { s << "Keyword: " << k << "\n"; },
					[&s](ast::PtrStyling& p) { s << "Ptr: " << p << "\n"; },
					[&s](ast::VarianceType& v) { s << "Variance: " << v << "\n"; },
					[&s](ast::RelationType& r) { s << "Relation: " << r << "\n"; },
					[&s](ast::VisibilityType& v) { s << "Visibility: " << v << "\n"; },
					[&s](ast::Sentinel&) { s << "Sentinel\n"; },
					[&s](ast::BindingType& b) { s << "BindType: " << b << "\n"; },
					[&s](ast::JumpType& j) { s << "Jump: " << j << "\n"; },
					[&s](ast::SequenceType& seq) { s << "SeqType: " << seq << "\n"; }
				), c);
			}
		), root);

		return s;
	}
}
