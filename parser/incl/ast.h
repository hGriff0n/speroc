#pragma once

#include "node_defs.h"

#include <optional>
#include <string>
#include <vector>
#include <memory>		// using std::unique_ptr as a cycle "breaker"


namespace spero::compiler::ast {
	//
	// Parent Nodes
	//
	struct Token {
	};

	struct Stmt : Token {
		std::vector<ptr<Annotation>> anots;
	};

	struct ValExpr : Stmt {
		bool is_mut;
		ptr<Type> type;
	};


	//
	// Literals
	//
	struct Literal : ValExpr {};
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


	//
	// Bindings
	//
	struct BasicBinding : Token {
		std::string name;
		BindingType type;
	};
	struct QualBinding : Token {
		std::vector<ptr<BasicBinding>> val;
	};
	

	//
	// Types
	//
	struct Type : Token {
		size_t id;
		bool is_mut;
	};
	struct BasicType : Type {
		ptr<BasicBinding> name;
	};
	struct GenType : BasicType {
		GenArray generics;
		PtrStyling pointer;
	};
	struct TupleType : Type {
		std::vector<ptr<Type>> val;
	};
	struct FuncType : Type {
		ptr<TupleType> args;
		ptr<Type> ret;
	};


	//
	// Atoms
	//
	struct TypeExt : Token {
		ptr<Sequence> args;				// optional, must be a tuple
		ptr<Sequence> body;				// must be a scope
	};
	struct Sequence : ValExpr {
		std::vector<astnode> vals;
		SequenceType seq;
	};
	struct FnCall : ValExpr {
		ptr<Token> caller;
		ptr<TypeExt> anon;			// optional
		ptr<Sequence> args;			// optional, must be a tuple
		ptr<Sequence> inst;			// optional, must be an array
	};
	struct FnBody : ValExpr {
		bool forward;
		ptr<Sequence> args;				// optional
		ptr<Type> ret;
		astnode body;
	};
	struct Range : ValExpr {
		astnode start, stop;
		astnode step;						// optional
	};


	//
	// Decorators
	//
	struct Annotation : Token {
		bool global;
		ptr<BasicBinding> name;
		ptr<Sequence> args;				// optional, must be a tuple
	};
	struct GenericPart : Token {
		ptr<BasicBinding> name;
		ptr<BasicType> type;
	};
	struct TypeGeneric : GenericPart {
		RelationType rel;
	};
	struct ValueGeneric : GenericPart {
		astnode val;					// optional
	};
	struct Case : Token {
		std::vector<ptr<Pattern>> vars;
		astnode expr;
	};
	struct ImportPart : Token {
		std::vector<ptr<ImportPiece>> val;
	};
	struct ImportPiece : Token {				// Represents '_'
	};
	struct ImportName : ImportPiece {
		ptr<BasicBinding> old;			// optional
		ptr<BasicBinding> name;
	};


	//
	// Assignment Parts
	//
	struct Adt : Token {
		ptr<BasicBinding> name;
		std::vector<ptr<BasicType>> args;
	};
	struct AssignPattern : Token {				// Represents '_'
	};
	struct AssignName : AssignPattern {
		ptr<BasicBinding> var;
	};
	struct AssignTuple : AssignPattern {
		std::vector<ptr<AssignPattern>> vars;
	};
	struct Pattern : Token {					// Represents '_'
		bool is_mut;
	};
	struct PTuple : Pattern {			// These two are confusing
		std::vector<ptr<Pattern>> val;
	};
	struct PNamed : Pattern {
		ptr<BasicBinding> name;
	};
	struct PAdt : Pattern {
		ptr<BasicBinding> name;			// must be a type
		ptr<PTuple> args;				// can't be false
	};


	//
	// Control
	//
	struct Branch : ValExpr {
		using IfBranch = std::pair<astnode, astnode>;
		std::vector<IfBranch> cases;
		astnode else_branch;				// optional
	};
	struct Loop : ValExpr {
		astnode body;
	};
	struct While : ValExpr {
		astnode test, body;
	};
	struct For : ValExpr {
		ptr<Pattern> pattern;
		astnode generator, body;
	};
	struct Match : ValExpr {
		astnode switch_val;
		std::vector<ptr<Case>> cases;
	};
	struct Jump : Loop {					// body is optional
		KeywordType jmp;
	};
	

	//
	// Stmts
	//
	struct ImplExpr : Stmt {
		ptr<QualBinding> type;				// Must be a type
	};
	struct ModDec : Stmt {
		ptr<QualBinding> module;			// Must be a var
	};
	struct ModImport : Stmt {
		std::vector<ptr<ImportPart>> parts;
	};
	struct Interface : Stmt {
		VisibilityType vis;
		ptr<AssignPattern> binding;
		GenArray generics;
		ptr<Type> type;						// optional for subtypes
	};
	struct TypeAssign : Interface {
		std::vector<token> cons;			// Must be an Adt or a Tuple Sequence
		ptr<Sequence> expr;
	};
	struct VarAssign : Interface {
		astnode expr;
	};
	struct Index : Stmt {
		bool deref;
		std::vector<astnode> elems;
		ptr<Type> type;						// optional
	};
	struct Infix : Stmt {
		astnode lhs, rhs;
		ptr<BasicBinding> op;
	};
}


namespace spero::util {
	#define PRETTY_PRINT(type) template<class Stream> \
	Stream& pretty_print(const spero::compiler::type& root, Stream& s, int depth = 0)

	#define PRETTY_PRINT_SCAFF(type) PRETTY_PRINT(type) { return s; }

	/*/
	// Normal Expressions
	PRETTY_PRINT(astnode) {
		return (Stream&)(s << root->pretty_fmt(depth));
	}

	// Bindings
	PRETTY_PRINT(ast::BasicBinding) {
		s << std::string(depth, ' ') << root.name << "(" << root.type << ")";
		return s;
	}
	PRETTY_PRINT(ast::QualBinding) {
		s << std::string(depth, ' ');

		for (auto binding : root.val) {
			pretty_print(binding, s, 0) << ":";
		}
		return s;
	}
	PRETTY_PRINT_SCAFF(ast::Type)

	// Decorators
	PRETTY_PRINT_SCAFF(ast::Annotation)
	PRETTY_PRINT_SCAFF(ast::FullType)
	PRETTY_PRINT_SCAFF(ast::TypeGeneric)
	PRETTY_PRINT_SCAFF(ast::ValueGeneric)
	PRETTY_PRINT_SCAFF(ast::TupleType)
	PRETTY_PRINT_SCAFF(ast::GenArray)
	PRETTY_PRINT_SCAFF(ast::Case)
	PRETTY_PRINT_SCAFF(ast::ImportPathPart)
	PRETTY_PRINT_SCAFF(ast::InstArray)

	// Assignment
	PRETTY_PRINT_SCAFF(ast::Adt)
	PRETTY_PRINT_SCAFF(ast::AssignTuple)
	PRETTY_PRINT_SCAFF(ast::AssignPattern)
	PRETTY_PRINT_SCAFF(ast::TypeExt)
	PRETTY_PRINT_SCAFF(ast::ImportRebind)
	PRETTY_PRINT_SCAFF(ast::PatternTuple)
	PRETTY_PRINT_SCAFF(ast::Any)
	PRETTY_PRINT_SCAFF(ast::NamedPattern)
	PRETTY_PRINT_SCAFF(ast::TuplePattern)
	PRETTY_PRINT_SCAFF(ast::AdtPattern)

	// Stack Values
	PRETTY_PRINT(tokens) {
		using namespace spero::compiler::ast;

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
			[&s, depth](const auto& v) { pretty_print(elem, s, depth); }
		), root);
		return s;
	}

	*/

	#undef PRETTY_PRINT_SCAFF
	#undef PRETTY_PRINT
}
