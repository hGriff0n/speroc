#pragma once

#include <variant>
#include <memory>
#include <deque>
#include <string>
#include "enum.h"
#include "SymTable.h"

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
	using Location = tao::pegtl::position;
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
	// BASE NODES: AST, VALEXPR, STMT
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
		Location loc;

		Ast(Location);

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
		Token(T val, Location loc) : Ast{ loc }, value{ val } {}

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");

		template<class T, class=std::enable_if_t<can_hold_v<T, token_type>>>
		bool holds() { return std::holds_alternative<T>(value); }
		template<class T, class=std::enable_if_t<can_hold_v<T, token_type>>>
		T get() { return std::get<T>(value); }
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

		Type(Location);
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
	struct Statement : Ast {
		std::deque<ptr<LocalAnnotation>> annots;

		Statement(Location);

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
	struct ValExpr : Statement {
		bool is_mut = false;
		ptr<Type> type;

		ValExpr(Location);

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

		Sequence(std::deque<ptr<T>> e, Location loc) : Inher{ loc }, elems{ std::move(e) } {}

		virtual Visitor& visit(Visitor&) =0;
	};



	//
	// LITERALS, ATOMS
	//
	// This section contains the nodes representing value-types that are compile-time recognizable
	//

	struct Literal : ValExpr {
		Literal(Location);
	};

	struct Bool : Literal {
		bool val;
		Bool(bool, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Byte : Literal {
		unsigned long val;
		Byte(std::string, int, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Float : Literal {
		double val;
		Float(std::string, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Int : Literal {
		long val;
		Int(std::string, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Char : Literal {
		char val;
		Char(char, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct String : ValExpr {
		std::string val;
		String(std::string, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Represents a value which will be provided in future analysis stages
	 *   mainly used for function forwarding to dot_control structures
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   generated - flag for whether the Future was created by the compiler (ala "dot function") or by the programmer (ie. "_")
	 */
	struct Future : ValExpr {
		bool generated;

		Future(bool, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * TODO: Write a description
	 */
	struct Tuple : Sequence<ValExpr> {
		Tuple(std::deque<ptr<ValExpr>>, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Array : Sequence<ValExpr> {
		Array(std::deque<ptr<ValExpr>>, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * TODO: Write a description
	 */
	struct Block : Sequence<Statement, ValExpr> {
		Block(std::deque<ptr<Statement>>, Location);

		//analysis::SymTable locals;

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Represents a lambda function body (ie. not a forwarded function)
	 *
	 * Extends: ValExpr
 	 *
	 * Exports:
	 *   forward - flag for whether the function body forwards arguments to the first expression
	 *   args    - tuple for function binding function arguments
	 *   body    - expression that the function evaluates
	 */
	struct Function : ValExpr {
		ptr<Tuple> args;
		ptr<ValExpr> body;

		Function(ptr<Tuple>, ptr<ValExpr>, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
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

		BasicBinding(std::string, BindingType, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;

		std::string toString();
	};

	/*
	 * Represents a complete Spero binding
	 *
	 * Extends: Seqeuence<BasicBinding, Ast>
	 */
	struct QualifiedBinding : Sequence<BasicBinding, Ast> {
		QualifiedBinding(ptr<BasicBinding>, Location);
		QualifiedBinding(std::deque<ptr<BasicBinding>>, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;

		std::string toString();
	};

	/*
	 * Base class for representing pattern matching expressions
	 *   Instance doubles for representing the any case
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   cap - descriptor of how the pattern is captured
	 */
	struct Pattern : Ast {
		CaptureType cap = CaptureType::NORM;

		Pattern(Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
	};

	/*
	 * Instance class for matching against a decomposed tuple
	 *
	 * Extends: Pattern
	 *
	 * Exports:
	 *   elems - collection of sub-patterns to match
	 */
	struct TuplePattern : Sequence<Pattern> {
		TuplePattern(std::deque<ptr<Pattern>>, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Instance class for binding to a variable in a pattern match
	 *
	 * Extends: Pattern
	 *
	 * Exports:
	 *   name - the value to bind to
	 */
	struct VarPattern : Pattern {
		ptr<QualifiedBinding> name;

		VarPattern(ptr<QualifiedBinding>, Location);

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
	struct AdtPattern : VarPattern {
		ptr<TuplePattern> args;

		AdtPattern(ptr<QualifiedBinding>, ptr<TuplePattern>, Location);

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
	struct ValPattern : Pattern {
		ptr<ValExpr> val;

		ValPattern(ptr<ValExpr>, Location);

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

		SourceType(ptr<QualifiedBinding>, Location);

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

		GenericType(ptr<QualifiedBinding>, ptr<Array>, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Type instance class that represents a tuple of types
	 *
	 * Extends: Sequence<Type>
	 *
	 * Exports:
	 *   elems - the collection of types that construct the tuple
	 */
	struct TupleType : Sequence<Type> {
		TupleType(std::deque<ptr<Type>>, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Type instance class that represents a function object
	 *
	 * Extends: Type
     *
	 * Exports:
	 *   args - the tuple type that represents the functions arguments
	 *   ret - the type returned by the function
	 */
	struct FunctionType : Type {
		ptr<TupleType> args;
		ptr<Type> ret;

		FunctionType(ptr<TupleType>, ptr<Type>, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Type instance class that represents a conjunction of several types
	 *
	 * Extends: Type
	 *
	 * Exports:
	 *   types - the collection of individual types
	 */
	struct AndType : Sequence<Type> {
		AndType(ptr<Type>, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Type instance class that represents a disjunction of several types
	 *
	 * Extends: Type
	 *
	 * Exports:
	 *   types - the collection of individual types
	 */
	struct OrType : Sequence<Type> {
		OrType(ptr<Type>, Location);

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

		Annotation(ptr<BasicBinding>, ptr<Tuple>, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
	};

	/*
	 * Represents an annotation instance that is attached to a statement
	 *
	 * Extends: LocalAnnotation
	 */
	struct LocalAnnotation : Annotation {
		LocalAnnotation(ptr<BasicBinding>, ptr<Tuple>, Location);

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

		Adt(ptr<BasicBinding>, ptr<TupleType>, Location);

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
		Branch(Location);

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

		Loop(ptr<ValExpr>, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
	};

	/*
	 * Represents a basic while loop construct
	 *
	 * Extends: Loop
	 *
	 * Exports:
	 *   test - the expression to test for loop termination
	 */
	struct While : Loop {
		ptr<ValExpr> test;

		While(ptr<ValExpr>, ptr<ValExpr>, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Represents a for loop construct
	 *
	 * Extends: Loop
	 *
	 * Exports:
	 *   pattern - the var pattern to hold iteration values
	 *   generator - the sequence that generates iteration values
	 */
	struct For : Loop {
		ptr<Pattern> pattern;
		ptr<ValExpr> generator;

		For(ptr<Pattern>, ptr<ValExpr>, ptr<ValExpr>, Location);

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
	/*struct IfElse : Branch {
		using Pair = std::pair<ptr<ValExpr>, ptr<ValExpr>>;

		std::deque<Pair> if_branches;
		ptr<ValExpr> else_branch;

		IfElse(ptr<ValExpr>, ptr<ValExpr>, Location);

		void addBranch(ptr<ValExpr>, ptr<ValExpr>);
		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};*/

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
		ptr<TuplePattern> vars;
		ptr<ValExpr> expr;
		ptr<ValExpr> if_guard;

		Case(ptr<TuplePattern>, ptr<ValExpr>, ptr<ValExpr>, Location);

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

		Match(ptr<ValExpr>, std::deque<ptr<Case>>, Location);

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
	struct Jump : Branch {
		KeywordType type;
		ptr<ValExpr> expr;

		Jump(KeywordType, ptr<ValExpr>, Location);
		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	//
	// STMTS
	//
	// This section represents all the compound statements in Spero not already presented
	//

	/*
	 * Represent an binary operator call
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   lhs - expression on the lhs of the operation
	 *   rhs - expression on the rhs of the operation
	 *   op  - operator that is being invoked
	 */
	struct BinOpCall : ValExpr {
		ptr<ValExpr> lhs;
		ptr<ValExpr> rhs;
		ptr<BasicBinding> op;
		//std::string op;

		//BinOpCall(ptr<ValExpr>, ptr<ValExpr>, std::string, Location);
		BinOpCall(ptr<ValExpr>, ptr<ValExpr>, ptr<BasicBinding>, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Stmt : Ast {};

}