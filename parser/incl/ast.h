#pragma once

#include "node_defs.h"

#include <optional>
#include <string>
#include <vector>
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
	};
	struct Stmt : Ast {
		std::vector<ptr<Annotation>> annots;

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
		std::vector<ptr<BasicBinding>> val;

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
		std::vector<ptr<Type>> val;
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
		std::vector<node> vals;
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
		std::vector<ptr<Pattern>> vars;
		value expr;
	};
	struct ImportPart : Ast {
		std::vector<ptr<ImportPiece>> val;
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
		std::vector<ptr<Type>> args;
	};
	struct AssignPattern : Ast {		// Represents '_'
	};
	struct AssignName : AssignPattern {
		ptr<BasicBinding> var;
	};
	struct AssignTuple : AssignPattern {
		std::vector<ptr<AssignPattern>> vars;
	};
	struct Pattern : Ast {				// Represents '_'
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
		using IfBranch = std::pair<value, value>;
		std::vector<IfBranch> cases;
		value else_branch;				// optional
	};
	struct Loop : ValExpr {
		value body;
	};
	struct While : ValExpr {
		value test, body;
	};
	struct For : ValExpr {
		ptr<Pattern> pattern;
		value generator, body;
	};
	struct Match : ValExpr {
		value switch_val;
		std::vector<ptr<Case>> cases;
	};
	struct Jump : ValExpr {
		value body;						// optional
		KeywordType jmp;
	};
	struct Wait : Jump {
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
		std::vector<ptr<ImportPart>> parts;
	};
	struct Interface : Stmt {
		VisibilityType vis;
		ptr<AssignPattern> binding;
		GenArray generics;
		ptr<Type> type;					// optional for subtypes
	};
	struct TypeAssign : Interface {		// type = inheritance
		std::vector<node> cons;			// Must be an Adt or a Tuple Sequence
		ptr<Block> expr;
	};
	struct VarAssign : Interface {
		value expr;
	};
	struct Index : ValExpr {
		std::vector<value> elems;
		ptr<Type> inf;					// optional (null = not known ???)
	};

	#undef PRETTY_PRINT
}
