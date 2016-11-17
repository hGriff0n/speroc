#pragma once

#include "node_defs.h"

#include <optional>
#include <string>

namespace spero::compiler::ast {
	//
	// Parent Nodes
	//
	struct Ast {
		Ast();

		virtual std::string pretty_print(size_t=0);
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
		virtual std::string pretty_print(size_t=0);
	};
	struct Stmt : Ast {
		std::deque<ptr<Annotation>> annots;

		Stmt();
	};
	struct ValExpr : Stmt {
		bool is_mut = false;
		UnaryType unop = UnaryType::NA;
		ptr<Type> type;

		ValExpr();
		virtual std::string pretty_print(size_t=0);
	};


	//
	// Literals
	//
	struct Bool : ValExpr {
		bool val;
		Bool(bool);

		virtual std::string pretty_print(size_t=0);
	};
	struct Byte : ValExpr {
		unsigned long val;
		Byte(const std::string&, int);

		virtual std::string pretty_print(size_t=0);
	};
	struct Float : ValExpr {
		double val;
		Float(const std::string&);

		virtual std::string pretty_print(size_t=0);
	};
	struct Int : ValExpr {
		int val;
		Int(const std::string&);

		virtual std::string pretty_print(size_t=0);
	};
	struct String : ValExpr {
		std::string val;
		String(const std::string&);

		virtual std::string pretty_print(size_t=0);
	};
	struct Char : ValExpr {
		char val;
		Char(char);

		virtual std::string pretty_print(size_t=0);
	};


	//
	// Bindings
	//
	struct BasicBinding : Ast {
		std::string name;
		BindingType type;

		BasicBinding(std::string, BindingType);
		virtual std::string pretty_print(size_t=0);
	};
	struct QualBinding : Ast {
		std::deque<ptr<BasicBinding>> val;

		QualBinding(ptr<BasicBinding>);
		QualBinding(std::deque<ptr<BasicBinding>>&);
		void add(ptr<BasicBinding>);
		virtual std::string pretty_print(size_t=0);
	};


	//
	// Types
	//
	struct Type : Ast {
		size_t id;
		bool is_mut = false;

		Type();
	};
	struct BasicType : Type {
		ptr<QualBinding> name;

		BasicType(ptr<BasicBinding>);
		BasicType(ptr<QualBinding>);
		virtual std::string pretty_print(size_t=0);
	};
	// NOTE: Unused
	struct GenType : BasicType {
		GenArray generics;
		PtrStyling pointer;
	};
	struct InstType : BasicType {
		ptr<Array> inst;
		PtrStyling pointer;

		InstType(ptr<QualBinding>, ptr<Array>, PtrStyling);
		virtual std::string pretty_print(size_t=0);
	};
	struct TupleType : Type {
		std::deque<ptr<Type>> val;

		TupleType(std::deque<ptr<Type>>&);
		virtual std::string pretty_print(size_t=0);
	};
	struct FuncType : Type {
		ptr<TupleType> args;
		ptr<Type> ret;

		FuncType(ptr<TupleType>, ptr<Type>);
		virtual std::string pretty_print(size_t=0);
	};


	//
	// Atoms
	//
	struct TypeExt : Ast {
		ptr<Tuple> cons;				// optional
		ptr<Block> body;

		TypeExt(ptr<Tuple>, ptr<Block>);
	};
	struct Sequence : ValExpr {
		std::deque<node> vals;
		Sequence(std::deque<node>&);
	};
	struct Tuple : Sequence {			// must accept values
		Tuple(std::deque<node>&);
		virtual std::string pretty_print(size_t=0);
	};
	struct Array : Sequence {
		Array(std::deque<node>&);
		virtual std::string pretty_print(size_t=0);
	};
	struct Block : Sequence {
		Block(std::deque<node>&);
		virtual std::string pretty_print(size_t=0);
	};
	struct FnCall : ValExpr {
		node caller;
		ptr<TypeExt> anon;				// optional
		ptr<Tuple> args;				// optional
		ptr<Array> inst;				// optional

		FnCall(node);
		FnCall(node, ptr<TypeExt>, ptr<Tuple>, ptr<Array>);
		virtual std::string pretty_print(size_t=0);
	};
	// NOTE: Not used
	struct Variable : ValExpr {
		ptr<QualBinding> var;
	};
	struct FnBody : ValExpr {
		bool forward;
		ptr<Tuple> args;				// null = not known
		ptr<Type> ret;					// null = not known
		value body;

		FnBody(value, bool);
		virtual std::string pretty_print(size_t=0);
	};
	struct Range : ValExpr {
		value start, stop;
		//value step;						// optional

		Range(value, value);
		virtual std::string pretty_print(size_t = 0);
	};


	//
	// Decorators
	//
	struct Annotation : Ast {
		bool global;
		ptr<BasicBinding> name;
		ptr<Tuple> args;				// optional

		Annotation(ptr<BasicBinding>, ptr<Tuple>, bool);
		virtual std::string pretty_print(size_t=0);
	};
	struct GenericPart : Ast {
		ptr<BasicBinding> name;
		ptr<Type> type;					// BasicType?

		GenericPart(ptr<BasicBinding>, ptr<Type>);
	};
	struct TypeGeneric : GenericPart {
		RelationType rel;
		VarianceType var;

		TypeGeneric(ptr<BasicBinding>, ptr<Type>, RelationType, VarianceType);
		virtual std::string pretty_print(size_t=0);
	};
	struct ValueGeneric : GenericPart {
		value val;						// optional

		ValueGeneric(ptr<BasicBinding>, ptr<Type>, value);
		virtual std::string pretty_print(size_t=0);
	};
	struct Case : Ast {
		std::deque<ptr<Pattern>> vars;
		value expr;

		Case(value);
		void setPattern(std::deque<ptr<Pattern>>&);
		virtual std::string pretty_print(size_t=0);	//temp?
	};
	struct ImportPart : Ast {
		std::deque<ptr<ImportPiece>> val;

		ImportPart(std::deque<ptr<ImportPiece>>&);
		void add(ptr<ImportPiece>);
		virtual std::string pretty_print(size_t = 0);
	};
	struct ImportPiece : Ast {			// Represents '_'
	};
	struct ImportName : ImportPiece {
		ptr<BasicBinding> old;			// optional
		ptr<BasicBinding> name;

		ImportName(ptr<BasicBinding>);
		ImportName(ptr<BasicBinding>, ptr<BasicBinding>);
		virtual std::string pretty_print(size_t = 0);

	};
	struct ImportGroup : ImportPiece {
		std::deque<ptr<ImportName>> imps;

		ImportGroup(std::deque<ptr<ImportName>>&);
		virtual std::string pretty_print(size_t = 0);
	};


	//
	// Assignment Parts
	//
	struct Adt : Ast {
		ptr<BasicBinding> name;
		ptr<TupleType> args;

		Adt(ptr<BasicBinding>, ptr<TupleType>);
		virtual std::string pretty_print(size_t=0);
	};
	struct AssignPattern : Ast {		// Represents '_'
	};
	struct AssignName : AssignPattern {
		ptr<BasicBinding> var;

		AssignName(ptr<BasicBinding>);
		virtual std::string pretty_print(size_t=0);
	};
	struct AssignTuple : AssignPattern {
		std::deque<ptr<AssignPattern>> vars;
		
		AssignTuple(std::deque<ptr<AssignPattern>>&);
		virtual std::string pretty_print(size_t=0);
	};
	struct Pattern : Ast {				// Represents '_'
		bool is_mut = false;
		virtual std::string pretty_print(size_t=0);
	};
	struct PTuple : Pattern {			// These two are confusing
		std::deque<ptr<Pattern>> val;

		PTuple(std::deque<ptr<Pattern>>&);
		virtual std::string pretty_print(size_t=0);
	};
	struct PNamed : Pattern {
		ptr<BasicBinding> name;

		PNamed(ptr<BasicBinding>);
		virtual std::string pretty_print(size_t=0);
	};
	struct PAdt : Pattern {
		ptr<BasicBinding> name;			// must be a type
		ptr<PTuple> args;				// can't be false

		PAdt(ptr<BasicBinding>, ptr<PTuple>);
		virtual std::string pretty_print(size_t=0);
	};
	struct Interface : Stmt {
		VisibilityType vis;
		ptr<AssignPattern> binding;
		GenArray generics;
		ptr<Type> type;					// optional for subtypes

		Interface(ptr<AssignPattern>, GenArray&, ptr<Type>);
		void setVisibility(VisibilityType);
		virtual std::string pretty_print(size_t = 0);
	};
	struct TypeAssign : Interface {		// type = inheritance
		std::deque<node> cons;			// Must be an Adt or a Tuple Sequence
		ptr<Block> body;

		TypeAssign(ptr<AssignPattern>, std::deque<node>&, GenArray&, ptr<Block>, ptr<Type>);
		virtual std::string pretty_print(size_t = 0);
	};
	struct VarAssign : Interface {		// type = inference
		value expr;

		VarAssign(ptr<AssignPattern>, GenArray&, value, ptr<Type>);
		virtual std::string pretty_print(size_t = 0);
	};


	//
	// Control
	//
	struct Branch : ValExpr {
		using IfBranch = std::pair<value, value>;
		std::deque<IfBranch> cases;
		value else_branch;				// optional

		Branch(value, value);
		void addIf(value, value);
		void addElse(value);

		virtual std::string pretty_print(size_t=0);
	};
	struct Loop : ValExpr {
		value body;

		Loop(value);
		virtual std::string pretty_print(size_t=0);
	};
	struct While : ValExpr {
		value test, body;

		While(value, value);
		virtual std::string pretty_print(size_t=0);
	};
	struct For : ValExpr {
		ptr<Pattern> pattern;
		value generator, body;

		For(ptr<Pattern>, value, value);
		virtual std::string pretty_print(size_t=0);
	};
	struct Match : ValExpr {
		value switch_val;
		std::deque<ptr<Case>> cases;

		Match(value, std::deque<ptr<Case>>&);
		virtual std::string pretty_print(size_t=0);
	};
	struct Jump : ValExpr {
		value body;						// optional
		KeywordType jmp;

		Jump(KeywordType, value);
	};
	struct Wait : Jump {
		Wait(value);
		virtual std::string pretty_print(size_t=0);
	};
	struct Break : Jump {
		Break(value);
		virtual std::string pretty_print(size_t=0);
	};
	struct Continue : Jump {
		Continue(value);
		virtual std::string pretty_print(size_t=0);
	};
	struct Return : Jump {
		Return(value);
		virtual std::string pretty_print(size_t=0);
	};
	struct YieldExpr : Jump {
		YieldExpr(value);
		virtual std::string pretty_print(size_t=0);
	};
	

	//
	// Stmts
	//
	struct ImplExpr : Stmt {
		ptr<QualBinding> type;			// Must be a type

		ImplExpr(ptr<QualBinding>);
		virtual std::string pretty_print(size_t = 0);
	};
	struct ModDec : Stmt {
		ptr<QualBinding> module;		// Must be a var

		ModDec(ptr<QualBinding>);
		virtual std::string pretty_print(size_t = 0);
	};
	struct ModImport : Stmt {
		std::deque<ptr<ImportPart>> parts;

		ModImport(std::deque<ptr<ImportPart>>&);
		virtual std::string pretty_print(size_t = 0);
	};
	struct Index : ValExpr {
		std::deque<value> elems;
		ptr<Type> inf;					// optional (null = not known ???)

		Index(value, value);
		void add(value);
		virtual std::string pretty_print(size_t = 0);
	};
}

namespace spero::parser {
	using Stack = std::deque<spero::compiler::node>;
}