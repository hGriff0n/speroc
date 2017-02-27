#pragma once

#include <variant>
#include <memory>
#include <deque>
#include <string>
#include "enum.h"


namespace spero::compiler {
	// Cut down on typing for unique_ptr
	template<class T> using ptr = std::unique_ptr<T>;

	/*
	 * Templates to check if a variant could hold a T
	 */
	template<class T, class V>
	struct can_hold : std::is_convertible<T, V> {};
	template<class T, class... V>
	struct can_hold<T, std::variant<V...>> : std::disjunction<can_hold<T, V>...> {};
	template<class T, class V>
	constexpr bool can_hold_v = can_hold<T, V>::value;
}

namespace spero::compiler::ast {
	/*
	 * Enums and other basic types
	 */
	using Sentinel = nullptr_t;
	BETTER_ENUM(KeywordType, char, LET, DEF, STATIC, MUT, DO,
				MOD, USE, MATCH, IF, ELSIF, ELSE, WHILE, FOR, LOOP,
				BREAK, CONT, YIELD, RET, WAIT, IMPL, F_IN)
	BETTER_ENUM(PtrStyling, char, PTR, REF, VIEW, NA)
	BETTER_ENUM(VarianceType, char, COVARIANT, CONTRAVARIANT, INVARIANT, VARIADIC)
	BETTER_ENUM(RelationType, char, IMPLS, NOT_IMPLS, SUBTYPE, SUPERTYPE, NA)
	BETTER_ENUM(VisibilityType, char, PUBLIC, PROTECTED, PRIVATE, STATIC)
	BETTER_ENUM(BindingType, char, TYPE, VARIABLE, OPERATOR)
	BETTER_ENUM(UnaryType, char, DEREF, NOT, MINUS, RESERVED, NA)
	BETTER_ENUM(CaptureType, char, NORM, MUT, REF, MUTREF)

	/*
	 * Forward declarations
	 */
	struct LocalAnnotation;
	struct Visitor;


	//
	// BASE NODES, AST, VALEXPR, STMT
	// 
	// This section contains the base nodes within the heirarchy
	//


	/*
	 * Base class for all ast nodes
	 *
	 * Specifies the required information that all ast nodes must contain
	 *
	 * Exports:
	 *   loc - Structure containing the location data for the node
	 *   visit - Polymorphic method to accept a visitor object for iteration
	 */
	struct Ast {
		struct Location {
			size_t byte, line;
			std::string src;
		} loc;

		Ast(Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
	};


	/*
	 * Base class for representing a token during parsing on the stack
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   value - a variant containing the actual token
	 */
	struct Token : Ast {
		using token_type = std::variant<KeywordType, PtrStyling, VarianceType, RelationType,
			VisibilityType, BindingType, UnaryType, CaptureType>;
		token_type value;

		template<class T, class = std::enable_if_t<can_hold_v<T, token_type>>>
		Token(T val, Ast::Location loc) : Ast{ loc }, value{ val } {}

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
	};


	/*
	 * Base class for representing a type within the heirarchy
	 *
	 * Extends Ast
	 *
	 * Exports:
	 *   id - number bound to the type that is assigned during analysis
	 *   is_mut - flag whether the type is mutable
	 *
	 * TODO: Figure out whether this works when considering analysis stages
	 */
	struct Type : Ast {
		size_t id;
		bool is_mut = false;

		Type(Ast::Location);
		virtual Visitor& visit(Visitor&);
	};


	/*
	 * Base class to represent any Spero statement
	 *   TODO: Differentiate from ValExpr
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   annotations - A collection of associated annotations
	 */
	struct Stmt : Ast {
		std::deque<ptr<LocalAnnotation>> annots;

		Stmt(Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
	};


	/*
	 * Base class to handle Spero statements that produce a value
	 *
	 * Extends: Stmt
	 *
	 * Exports:
	 *   is_mut - flag whether the produced value is mutable
	 *   unop   - token for any unary operations applied
	 *   type   - the marked type for the expression
	 *
	 * TODO:
	 *   type will need to be changed in the future to reduce luggage
	 */
	struct ValExpr : Stmt {
		bool is_mut = false;
		ptr<Type> type;

		ValExpr(Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
	};


	/*
	 * Base class to represent any sequenced collection of ast nodes
	 * The second template parameter is used to control the inheritance tree
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   exprs - collection of nodes that satisfy the sequence (stored front->back)
	 */
	template<class T, class Inher=T>
	struct Sequence : Inher {
		std::deque<ptr<T>> elems;

		Sequence(std::deque<ptr<T>> e, Ast::Location loc) : Inher{ loc }, elems{ std::move(e) } {}

		virtual Visitor& visit(Visitor&);
	};



	//
	// LITERALS, ATOMS
	//
	// This section contains the nodes representing value-types that are compile-time recognizable
	//


	struct Bool : ValExpr {
		bool val;
		Bool(bool, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Byte : ValExpr {
		unsigned long val;
		Byte(std::string, int, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Float : ValExpr {
		double val;
		Float(std::string, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Int : ValExpr {
		long val;
		Int(std::string, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Char : ValExpr {
		char val;
		Char(char, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct String : ValExpr {
		std::string val;
		String(std::string, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Tuple : Sequence<ValExpr> {
		Tuple(std::deque<ptr<ValExpr>>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Array : Sequence<ValExpr> {
		Array(std::deque<ptr<ValExpr>>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Block : Sequence<Stmt, ValExpr> {
		Block(std::deque<ptr<Stmt>>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	// 
	// NAMES, BINDINGS, PATTERNS
	// 
	// This section contains the nodes representing bindings, names, and patterns
	// 

	/*
	 * Represents the basic block of a Spero binding
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   name - the string representation of the binding
	 *   type - is the string a variable, type, or operator
	 *
	 * TODO:
	 *   Split this class into Operator/Variable/Type binding classes ???
	 */
	struct BasicBinding : Ast {
		std::string name;
		BindingType type;

		BasicBinding(std::string, BindingType, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Represents a complete Spero binding
	 *
	 * Extends: Seqeuence<BasicBinding, Ast>
	 */
	struct QualifiedBinding : Sequence<BasicBinding, Ast> {
		QualifiedBinding(ptr<BasicBinding>, Ast::Location);
		QualifiedBinding(std::deque<ptr<BasicBinding>>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Basic class that represents a variable name
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   name - qualified binding that represents the variable
	 */
	struct Variable : ValExpr {
		ptr<QualifiedBinding> name;

		Variable(ptr<QualifiedBinding>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Base class for representing an assignable binding
	 *   Instance doubles for representing the any case
	 *
	 * Extends: Ast
	 *
	 * Note: Instance usage is currently deprecated
	 */
	struct AssignPattern : Ast {
		AssignPattern(Ast::Location);
		virtual Visitor& visit(Visitor&);
	};


	/*
	 * Represents a singular named binding in an assignment context
	 *
	 * Extends: AssignPattern
	 *
	 * Exports:
	 *   name - binding to assign to
	 */
	struct AssignName : AssignPattern {
		ptr<BasicBinding> var;

		AssignName(ptr<BasicBinding>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Represents a decomposed tuple assignment
	 *
	 * Extends: AssignPattern
	 *
	 * Exports:
	 *   vars - a collection of AssignPattern to match against
	 */
	struct AssignTuple : Sequence<AssignPattern> {
		AssignTuple(std::deque<ptr<AssignPattern>>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Base class for representing pattern matching expressions
	 *   Instance doubles for representing the any case
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   is_mut - flag for whether bound value should be mutable
	 */
	struct Pattern : Ast {
		CaptureType cap = CaptureType::NORM;

		Pattern(Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
	};


	/*
	 * Instance class for matching against a decomposed tuple
	 *
	 * Extends: Pattern
	 *
	 * Exports:
	 *   ptns - collection of sub-patterns to match
	 */
	struct PTuple : Sequence<Pattern> {
		PTuple(std::deque<ptr<Pattern>>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Instance class for binding a value in a pattern match
	 *
	 * Extends: Pattern
	 *
	 * Exports:
	 *   name - the value to bind to
	 */
	struct PNamed : Pattern {
		ptr<BasicBinding> name;

		PNamed(ptr<BasicBinding>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
	};


	/*
	 * Instance class for matching against a decomposed ADT
	 *
	 * Extends: PNamed
	 *   Uses name as the ADT type to match againts
	 *
	 * Exports:
	 *   args - the tuple to match values against
	 */
	struct PAdt : PNamed {
		ptr<PTuple> args;

		PAdt(ptr<BasicBinding>, ptr<PTuple>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Instance class for matching against a value
	 *
	 * Extends: Pattern
	 *
	 * Exports:
	 *   value - value to match against
	 */
	struct PVal : Pattern {
		ptr<ValExpr> val;

		PVal(ptr<ValExpr>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	//
	// TYPES
	//
	// This section contains the nodes to collect information about types in source files
	//

	/*
	 * Basic class for type information obtained directly from the source file
	 *
	 * Extends: Type
	 *
	 * Exports:
	 *   name - qualified binding that the type is refering to
	 *   pointer - any pointer/view styling applied to the type
	 */
	struct SourceType : Type {
		ptr<QualifiedBinding> name;
		PtrStyling _ptr;

		SourceType(ptr<BasicBinding>, PtrStyling, Ast::Location);
		SourceType(ptr<QualifiedBinding>, PtrStyling, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
	};


	/*
	 * Basic instance class for generic instantiated types
	 *
	 * Extends: SourceType
	 *
	 * Exports:
	 *   inst - an array of generic parameters
	 */
	struct GenericType : SourceType {
		ptr<Array> inst;

		GenericType(ptr<QualifiedBinding>, ptr<Array>, PtrStyling, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Type instance class that represents a tuple of types
	 *
	 * Extends: SourceType
	 *
	 * Exports:
	 *   elems - the collection of types that construct the tuple
	 */
	struct TupleType : Sequence<Type> {
		TupleType(std::deque<ptr<Type>>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Type instance class that represents a function object
	 *
	 * Extends: SourceType

	 * Exports:
	 *   args - the tuple type that represents the functions arguments
	 *   ret - the type returned by the function
	 */
	struct FuncType : Type {
		ptr<TupleType> args;
		ptr<Type> ret;

		FuncType(ptr<TupleType>, ptr<Type>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	//
	// DECORATIONS, ANNOTATION, GENERIC
	//
	// This section contains all the minor nodes that round out specific cases in Spero
	//


	/*
	 * Represents a globally applied annotation (one that is not attached to a statement)
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   name - binding that the annotation is associated to
	 *   args - arguments provided to the annotation
	 */
	struct Annotation : Ast {
		ptr<BasicBinding> name;
		ptr<Tuple> args;

		Annotation(ptr<BasicBinding>, ptr<Tuple>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
	};


	/*
	 * Represents an annotation instance that is attached to a statement
	 *
	 * Extends: LocalAnnotation
	 */
	struct LocalAnnotation : Annotation {
		LocalAnnotation(ptr<BasicBinding>, ptr<Tuple>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Base class for pieces of generic declarations
	 *
	 * Extends: Ast
	 *   Reuses `prettyPrint` definition
	 *
	 * Exports:
	 *   name - name to bind the generic value to for the expression context
	 *   type - evaluable type of the binding
	 */
	struct GenericPart : Ast {
		ptr<BasicBinding> name;
		ptr<Type> type;

		GenericPart(ptr<BasicBinding>, ptr<Type>, Ast::Location);
		virtual Visitor& visit(Visitor&);
	};


	/*
	 * Instance class for generics on typing information
	 *
	 * Extends: GenericPart
	 *
	 * Exports:
	 *   rel - required relationship of the instance field to a declared type
	 *   var - flag for the variance of the parent instance in relation to the instance field
	 */
	struct TypeGeneric : GenericPart {
		RelationType rel;
		VarianceType var;

		TypeGeneric(ptr<BasicBinding>, ptr<Type>, RelationType, VarianceType, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Instance class for generics on value information
	 *
	 * Extends: GenericPart
	 *
	 * Exports:
	 *   value - default value for the generic field if none is provided or inferrable
	 */
	struct ValueGeneric : GenericPart {
		ptr<ValExpr> value;

		ValueGeneric(ptr<BasicBinding>, ptr<Type>, ptr<ValExpr>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Represents an anonymous type extension
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   cons - the primary constructor for the extended type
	 *   body - the type's body
	 */
	struct TypeExt : Ast {
		ptr<Tuple> cons;
		ptr<Block> body;

		TypeExt(ptr<Block>, Ast::Location);
		TypeExt(ptr<Tuple>, ptr<Block>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Class that represents a single statement within a match construct
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   vars - the pattern set to match against
	 *   expr - the expression to run on match success
	 *   if_guard - optional if statement to further control matching
	 */
	struct Case : ValExpr {
		ptr<PTuple> vars;
		ptr<ValExpr> expr;
		ptr<ValExpr> if_guard;

		Case(ptr<PTuple>, ptr<ValExpr>, ptr<ValExpr>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Base class for representing import pieces
	 *   Instance doubles for representing the any case
	 *
	 * Extends: Ast
	 */
	struct ImportPiece : Ast {
		ImportPiece(Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
	};


	/*
	 * Instance class for representing named import pieces
	 *   Also handles rebinding if a second name is provided
	 *
	 * Extends: ImportPiece
	 *
	 * Exports:
	 *   name     - binding that will be imported into the current context
	 *   new_name - binding that the old value will be bound under
	 *   generic_inst - any generic fields for grabbing certain typings
	 */
	struct ImportName : ImportPiece {
		ptr<BasicBinding> name;
		ptr<BasicBinding> new_name;
		ptr<Array> generic_inst;

		ImportName(ptr<BasicBinding>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Represents a collection of import pieces (ala. '{...}')
	 *
	 * Extends: ImportPiece
	 *
	 * Exports:
	 *   imps - the collection of pieces to import
	 */
	struct ImportGroup : Sequence<ImportPiece> {
		ImportGroup(std::deque<ptr<ImportPiece>>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Represents an Algebraic Data Type constructor
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   name - the type name of the constructor
	 *   args - the collection of types that the constructor takes
	 */
	struct Adt : Ast {
		ptr<BasicBinding> name;
		ptr<TupleType> args;

		Adt(ptr<BasicBinding>, ptr<TupleType>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Represents a value which will be provided in future analysis stages
	 *   mainly used for function forwarding to dot_control structures
	 *
	 * Extends: ValExpr
	 *
	 * Expors:
	 *   forwarded_from_fn - flag for whether the Future was created to represent a forwarded argument
	 */
	struct Future : ValExpr {
		bool forwarded_from_fn;

		Future(bool, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	
	//
	// CONTROL
	//
	// This section contains the nodes for all Spero control structures
	//

	/*
	 * Base class for all nodes that represents operations requiring branch/jump instructions
	 *
	 * Extends: ValExpr
	 */
	struct Branch : ValExpr {
		Branch(Ast::Location);

		virtual Visitor& visit(Visitor&);
	};


	/*
	 * An infinite loop that just repeats the body
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   body - the body of the loop
	 */
	struct Loop : Branch {
		ptr<ValExpr> body;
		
		Loop(ptr<ValExpr>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
	};


	/*
	 * Represents a basic while loop construct
	 *
	 * Extends: ValExpr
	 * 
	 * Exports:
	 *   test - the expression to test for loop termination
	 *   body - the body of the loop
	 */
	struct While : Loop {
		ptr<ValExpr> test;

		While(ptr<ValExpr>, ptr<ValExpr>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Represents a for loop construct
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   pattern - the var pattern to hold iteration values
	 *   generator - the sequence that generates iteration values
	 *   body - the body of the loop
	 */
	struct For : Loop {
		ptr<Pattern> pattern;
		ptr<ValExpr> generator;

		For(ptr<Pattern>, ptr<ValExpr>, ptr<ValExpr>, Ast::Location);

		virtual Visitor& visit(Visitor&) final;
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Represents a sequence of branching constructs
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   if_branches - sequence of conditional branching
	 *   else_branch - optional fall-through case
	 */
	struct IfElse : Branch {
		using Pair = std::pair<ptr<ValExpr>, ptr<ValExpr>>;

		std::deque<Pair> if_branches;
		ptr<ValExpr> else_branch;

		IfElse(ptr<ValExpr>, ptr<ValExpr>, Ast::Location);

		void addBranch(ptr<ValExpr>, ptr<ValExpr>);
		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Represents a pattern match construct
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   switch_expr - value to match against
	 *   cases - sequence of case statements to use in matching
	 */
	struct Match : Branch {
		ptr<ValExpr> switch_expr;
		std::deque<ptr<Case>> cases;

		Match(ptr<ValExpr>, std::deque<ptr<Case>>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Base class for all simple "branch" expressions
	 *
	 * Extends: Branch
	 *
	 * Exports:
	 *   expr - evaluable body
	 */
	struct JumpExpr : Branch {
		ptr<ValExpr> expr;

		JumpExpr(ptr<ValExpr>, Ast::Location);
		virtual Visitor& visit(Visitor&);
	};

	struct Wait : JumpExpr {
		Wait(ptr<ValExpr>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Break : JumpExpr {
		Break(ptr<ValExpr>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Continue : JumpExpr {
		Continue(ptr<ValExpr>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Return : JumpExpr {
		Return(ptr<ValExpr>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct YieldRet : JumpExpr {
		YieldRet(ptr<ValExpr>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	//
	// STMTS
	//
	// This section represents all the compound statements in Spero not already presented
	//

	/*
	 * Represents a function lambda
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   forward - flag for whether the function body forwards arguments to the first expression
	 *   args    - tuple for function binding function arguments
	 *   ret     - return type of the function (Note: type = args.type -> ret)
	 *   body    - expression that the function evaluates
	 */
	struct FnBody : ValExpr {
		bool forward;
		ptr<Tuple> args;
		ptr<Type> ret;
		ptr<ValExpr> body;

		FnBody(ptr<ValExpr>, bool, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
	};


	/*
	 * Represents a complete step of fuction sequencing
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   caller - expression that applies the function sequence
	 *   anon   - anonymous type extension section
	 *   args   - argument tuple section
	 *   inst   - generic instantiation section
	 */
	struct FnCall : ValExpr {
		ptr<ValExpr> caller;
		ptr<TypeExt> anon;
		ptr<Tuple> args;
		ptr<Array> inst;

		FnCall(ptr<ValExpr>, ptr<TypeExt>, ptr<Tuple>, ptr<Array>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Represents a range object construction
	 * 
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   start - range beginning value
	 *   stop  - range end value
	 */
	struct Range : ValExpr {
		ptr<ValExpr> start, stop;
		// ptr<ValExpr> step;

		Range(ptr<ValExpr>, ptr<ValExpr>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * A node to correctly handle chaining of unary applications
	 *
	 * Extends: ValExpr
	 */
	struct UnaryOpApp : ValExpr {
		UnaryType op;
		ptr<ValExpr> expr;

		UnaryOpApp(ptr<ValExpr>, UnaryType, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	using GenArray = std::deque<ptr<GenericPart>>;
	/*
	 * Base case for handling assignment statements
	 *   Instance also handles interface specification
	 *
	 * Extends: Stmt
	 *
	 * Exports:
	 *   vis - visibility of the created binding
	 *   name - pattern that represents the binding
	 *   generics - collection of generic parts for the assignment
	 *   type - a field holding type information for the interface/assignment
	 */
	struct Interface : Stmt {
		VisibilityType vis;
		ptr<AssignPattern> name;
		GenArray generics;
		ptr<Type> type;

		Interface(ptr<AssignPattern>, GenArray, ptr<Type>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
	};


	/*
	 * Instance class for specifically handling type binding
	 *
	 * Extends: Interface
	 *   Uses `type` for inheritance
	 *
	 * Exports:
	 *   cons - list of adt and primary constructors
	 *   body - type body
	 */
	struct TypeAssign : Interface {
		std::deque<ptr<Ast>> cons;
		ptr<Block> body;

		TypeAssign(ptr<AssignPattern>, std::deque<ptr<Ast>>, GenArray, ptr<Block>, ptr<Type>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Instance class for handling value assignment to variables and operators
	 *
	 * Extends: Interface
	 *   Uses `type` for static checking
	 *
	 * Exports:
	 *   expr - value to be assigned to the binding
	 */
	struct VarAssign : Interface {
		ptr<ValExpr> expr;

		VarAssign(ptr<AssignPattern>, GenArray, ptr<ValExpr>, ptr<Type>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Represents a scoped binding assignment, where a variable is declared for use in
	 *   a single expression and is only visible within that context
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   binding - the assignment binding the variable to some value
	 *   expr - the statement in which the binding is visible
	 */
	struct InAssign : ValExpr {
		ptr<VarAssign> binding;
		ptr<ValExpr> expr;

		InAssign(ptr<ValExpr>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	
	/*
	 * Represents an implements expression within a type body used to import functions
	 *   into the types interface. Almost entirely similar to standard OOP-inheritance
	 *
	 * Extends: Stmt
	 *
	 * Exports:
	 *   type - the type that is being implemented
	 *   impls - an (optional) block that may contain implementations for imported functions
	 *     NOTE: This may change to a required component
	 */
	struct ImplExpr : Stmt {
		ptr<GenericType> type;
		ptr<Block> impls;

		ImplExpr(ptr<GenericType>, ptr<Block>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	
	/*
	 * Represents a module declaration expression
	 *
	 * Extends: Stmt
	 *
	 * Exports:
	 *   module - the name of the created module
	 */
	struct ModDec : Stmt {
		ptr<QualifiedBinding> module;

		ModDec(ptr<QualifiedBinding>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Represents a module import expression
	 *
	 * Extends: Sequence<ImportPiece,Stmt>
	 */
	struct ModImport : Sequence<ImportPiece, Stmt> {
		ModImport(std::deque<ptr<ImportPiece>>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Represents a sequence of values where the each one is used to index the prior
	 *
	 * Extends: Sequence<ValExpr>
	 */
	struct Index : Sequence<ValExpr> {
		Index(ptr<ValExpr>, ptr<ValExpr>, Ast::Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};
}

namespace spero::parser {
	using Stack = std::deque<compiler::ptr<compiler::ast::Ast>>;
}