#pragma once

#include "node_defs.h"

#include <optional>
#include <string>
#include <deque>
#include <memory>		// using std::unique_ptr as a cycle "breaker"

namespace spero::compiler::ast {
	// Member templates can't be virtual
	#define PRETTY_PRINT virtual std::string pretty_print(size_t=0)

	//
	// Parent Nodes
	//
	struct Ast {
		Ast();

		PRETTY_PRINT;
	};
	struct Token : Ast {
		std::variant<KeywordType, PtrStyling, VarianceType, RelationType, VisibilityType, BindingType, UnaryType> val;

		// TODO: Try to get this down to one 'enable_if'd constructor
		Token(KeywordType);
		Token(PtrStyling);
		Token(VarianceType);
		Token(RelationType);
		Token(VisibilityType);
		Token(BindingType);
		Token(UnaryType);
		PRETTY_PRINT;
	};
	struct Stmt : Ast {
		std::deque<ptr<Annotation>> annots;

		Stmt();
	};
	struct ValExpr : Stmt {
		bool is_mut;
		UnaryType unop = UnaryType::NA;
		ptr<Type> type;

		ValExpr();
	};


	//
	// Literals
	//
	struct Bool : ValExpr {
		bool val;
		Bool(bool);

		PRETTY_PRINT;
	};
	struct Byte : ValExpr {
		unsigned long val;
		Byte(const std::string&, int);

		PRETTY_PRINT;
	};
	struct Float : ValExpr {
		double val;
		Float(const std::string&);

		PRETTY_PRINT;
	};
	struct Int : ValExpr {
		int val;
		Int(const std::string&);

		PRETTY_PRINT;
	};
	struct String : ValExpr {
		std::string val;
		String(const std::string&);

		PRETTY_PRINT;
	};
	struct Char : ValExpr {
		char val;
		Char(char);

		PRETTY_PRINT;
	};


	//
	// Bindings
	//
	struct BasicBinding : Ast {
		std::string name;
		BindingType type;

		BasicBinding(std::string, BindingType);
		PRETTY_PRINT;
	};
	struct QualBinding : Ast {
		std::deque<ptr<BasicBinding>> val;

		QualBinding(ptr<BasicBinding>);
		void add(ptr<BasicBinding>);
		PRETTY_PRINT;
	};


	//
	// Types
	//
	struct Type : Ast {
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
	struct InstType : BasicType {
		ptr<Array> inst;
	};
	struct TupleType : Type {
		std::deque<ptr<Type>> val;
	};
	struct FuncType : Type {
		ptr<TupleType> args;
		ptr<Type> ret;
	};


	//
	// Atoms
	//
	struct TypeExt : Ast {
		ptr<Tuple> args;				// optional
		ptr<Block> body;
	};
	struct Sequence : ValExpr {
		std::deque<node> vals;
	};
	struct Tuple : Sequence {			// must accept values
	};
	struct Array : Sequence {
	};
	struct Block : Sequence {
	};
	struct FnCall : ValExpr {
		node caller;
		ptr<TypeExt> anon;				// optional
		ptr<Tuple> args;				// optional
		ptr<Array> inst;				// optional

		FnCall(node);
		FnCall(node, ptr<TypeExt>, ptr<Tuple>, ptr<Array>);
		PRETTY_PRINT;
	};
	struct Variable : ValExpr {
		ptr<QualBinding> var;
	};
	struct FnBody : ValExpr {
		bool forward;
		ptr<Tuple> args;				// null = not known
		ptr<Type> ret;					// null = not known
		value body;
	};
	struct Range : ValExpr {
		value start, stop;
		value step;						// optional
	};


	//
	// Decorators
	//
	struct Annotation : Ast {
		bool global;
		ptr<BasicBinding> name;
		ptr<Tuple> args;				// optional
	};
	struct GenericPart : Ast {
		ptr<BasicBinding> name;
		ptr<Type> type;					// BasicType?
	};
	struct TypeGeneric : GenericPart {
		RelationType rel;
	};
	struct ValueGeneric : GenericPart {
		value val;						// optional
	};
	struct Case : Ast {
		std::deque<ptr<Pattern>> vars;
		value expr;
	};
	struct ImportPart : Ast {
		std::deque<ptr<ImportPiece>> val;
	};
	struct ImportPiece : Ast {			// Represents '_'
	};
	struct ImportName : ImportPiece {
		ptr<BasicBinding> old;			// optional
		ptr<BasicBinding> name;
	};


	//
	// Assignment Parts
	//
	struct Adt : Ast {
		ptr<BasicBinding> name;
		std::deque<ptr<Type>> args;
	};
	struct AssignPattern : Ast {		// Represents '_'
	};
	struct AssignName : AssignPattern {
		ptr<BasicBinding> var;
	};
	struct AssignTuple : AssignPattern {
		std::deque<ptr<AssignPattern>> vars;
	};
	struct Pattern : Ast {				// Represents '_'
		bool is_mut;
	};
	struct PTuple : Pattern {			// These two are confusing
		std::deque<ptr<Pattern>> val;
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
		using IfBranch = std::pair<value, value>;
		std::deque<IfBranch> cases;
		value else_branch;				// optional
	};
	struct Loop : ValExpr {
		value body;

		Loop(value);
		PRETTY_PRINT;
	};
	struct While : ValExpr {
		value test, body;

		While(value, value);
		PRETTY_PRINT;
	};
	struct For : ValExpr {
		ptr<Pattern> pattern;
		value generator, body;
	};
	struct Match : ValExpr {
		value switch_val;
		std::deque<ptr<Case>> cases;
	};
	struct Jump : ValExpr {
		value body;						// optional
		KeywordType jmp;

		Jump(KeywordType, value);
	};
	struct Wait : Jump {
		Wait(value);

		PRETTY_PRINT;
	};
	struct Break : Jump {
	};
	struct Continue : Jump {
	};
	struct Return : Jump {
	};
	struct Yield : Jump {
	};
	

	//
	// Stmts
	//
	struct ImplExpr : Stmt {
		ptr<QualBinding> type;			// Must be a type
	};
	struct ModDec : Stmt {
		ptr<QualBinding> module;		// Must be a var
	};
	struct ModImport : Stmt {
		std::deque<ptr<ImportPart>> parts;
	};
	struct Interface : Stmt {
		VisibilityType vis;
		ptr<AssignPattern> binding;
		GenArray generics;
		ptr<Type> type;					// optional for subtypes
	};
	struct TypeAssign : Interface {		// type = inheritance
		std::deque<node> cons;			// Must be an Adt or a Tuple Sequence
		ptr<Block> expr;
	};
	struct VarAssign : Interface {
		value expr;
	};
	struct Index : ValExpr {
		std::deque<value> elems;
		ptr<Type> inf;					// optional (null = not known ???)
	};

	#undef PRETTY_PRINT
}

namespace spero::parser {
	using Stack = std::deque<spero::compiler::node>;
}