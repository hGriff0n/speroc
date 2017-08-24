#pragma once

#include <variant>
#include <deque>
#include <string>

#include "enum.h"
#include "base.h"

namespace spero::compiler {
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
	BETTER_ENUM(VisibilityType, char, PUBLIC, PROTECTED, PRIVATE)
	BETTER_ENUM(BindingType, char, TYPE, VARIABLE, OPERATOR)
	BETTER_ENUM(CaptureType, char, NORM, MUT, REF, MUTREF)


	/*
	* Forward declarations
	*/
	struct LocalAnnotation;


	//
	// BASE NODES: AST, VALEXPR, STMT
	// 
	// This section contains the base nodes within the heirarchy
	//

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
			VisibilityType, BindingType, CaptureType>;
		token_type value;

		template<class T, class = std::enable_if_t<can_hold_v<T, token_type>>>
		Token(T val, Location loc) : Ast{ loc }, value{ val } {}

		virtual void accept(Visitor&);
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
		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
	};


	/*
	 * Base class to handle Spero statements that produce a value
	 *
	 * Extends: Statement
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

		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&) =0;
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

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Byte : Literal {
		unsigned long val;
		Byte(std::string, int, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Float : Literal {
		double val;
		Float(std::string, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Int : Literal {
		long val;
		Int(std::string, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Char : Literal {
		char val;
		Char(char, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct String : ValExpr {
		std::string val;
		String(std::string, Location);

		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * TODO: Write a description
	 */
	struct Tuple : Sequence<ValExpr> {
		Tuple(std::deque<ptr<ValExpr>>, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	struct Array : Sequence<ValExpr> {
		Array(std::deque<ptr<ValExpr>>, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * TODO: Write a description
	 */
	struct Block : Sequence<Statement, ValExpr> {
		analysis::SymTable locals;

		Block(std::deque<ptr<Statement>>, Location);

		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;

		std::string toString();
	};

	/*
	 * Base class for representing pattern matching expressions
	 *   Instance doubles for representing the "any" case
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   cap - descriptor of how the pattern is captured
	 */
	struct Pattern : Ast {
		CaptureType cap = CaptureType::NORM;

		Pattern(Location);

		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	/*
	 * Base class for representing an assignable binding
	 *   Instance doubles for representing the "drop" case
	 *
	 * Extends: Ast
	 *
	 * Note: Instance usage is currently deprecated
	 */
	struct AssignPattern : Ast {
		AssignPattern(Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
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

		AssignName(ptr<BasicBinding>, Location);

		virtual void accept(Visitor&);
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
		AssignTuple(std::deque<ptr<AssignPattern>>, Location);

		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
	};

	/*
	 * Represents an annotation instance that is attached to a statement
	 *
	 * Extends: LocalAnnotation
	 */
	struct LocalAnnotation : Annotation {
		LocalAnnotation(ptr<BasicBinding>, ptr<Tuple>, Location);

		virtual void accept(Visitor&);
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
	 *   rel - required relationship of the instance field to a declared type
	 */
	struct GenericPart : Ast {
		ptr<BasicBinding> name;
		ptr<Type> type;
		RelationType rel;

		GenericPart(ptr<BasicBinding>, ptr<Type>, RelationType, Location);
		virtual void accept(Visitor&);
	};

	/*
	 * Instance class for generics that require a type
	 *
	 * Extends: GenericPart
	 *
	 * Exports:
	 *   variance - enum specifying the variance of the parent instance in relation to generic type
	 *   variadic - flag for whether the type specifies a variadic collection
	 */
	struct TypeGeneric : GenericPart {
		VarianceType variance;
		bool variadic;

		TypeGeneric(ptr<BasicBinding>, ptr<Type>, RelationType, VarianceType, bool, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Instance class for generics that require a value
	 *
	 * Extends: GenericPart
	 *
	 * Exports:
	 *   value - default value for the generic field if none is provided or inferrable
	 */
	struct ValueGeneric : GenericPart {
		//ptr<ValExpr> value;

		//ValueGeneric(ptr<BasicBinding>, ptr<Type>, ptr<ValExpr>, Location);
		ValueGeneric(ptr<BasicBinding>, ptr<Type>, RelationType, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Collection class for generic declaration information
	 *
	 * Extends: Sequence<GenericPart, Ast>
	 */
	struct GenericArray : Sequence<GenericPart, Ast> {
		GenericArray(std::deque<ptr<GenericPart>>, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Ast subtype to annotate a type constructor
	 *   Currently empty
	 *
	 * Extends: Ast
	 */
	struct Constructor : Ast {
		Constructor(Location);

		virtual void accept(Visitor&) = 0;
	};

	/*
	 * Represents an Algebraic Data Type constructor
	 *
	 * Extends: Constructor
	 *
	 * Exports:
	 *   name - the type name of the constructor
	 *   args - the collection of types that the constructor takes
	 */
	struct Adt : Constructor {
		ptr<BasicBinding> name;
		ptr<TupleType> args;

		Adt(ptr<BasicBinding>, ptr<TupleType>, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Represents a function/constructor argument
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   name - binding mapped to the functional value
	 *   typ - acceptable impl boundary for passed values
	 */
	struct Argument : Ast {
		ptr<BasicBinding> name;
		ptr<Type> typ;

		Argument(ptr<BasicBinding>, ptr<Type>, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Represents a simple argument tuple for a type constructor
	 *
	 * Extends: Constructor
	 */
	struct ArgTuple : Sequence<Argument, Constructor> {
		ArgTuple(std::deque<ptr<Argument>>, Location);

		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
	};

	/*
	 * Represents a basic while loop construct
	 *
	 * Extends: Loop
	 *
	 * Exports:
	 *   test - the expression to test for loop termination
	 *
	 * TODO: Look into having a common parent with IfBranch
	 */
	struct While : Loop {
		ptr<ValExpr> test;

		While(ptr<ValExpr>, ptr<ValExpr>, Location);

		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&) final;
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Represents a single branch case within an if-elsif-else statement
	 *
	 * Extends: Branch
	 *
	 * Exports:
	 *   test - expression to test for branching
	 *   body - expression to evaluate if test succeeds
	 */
	struct IfBranch : Branch {
		ptr<ValExpr> test;
		ptr<ValExpr> body;
		bool elsif;

		IfBranch(ptr<ValExpr>, ptr<ValExpr>, bool, Location);

		virtual void accept(Visitor&) final;
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Represents a sequence of branching constructs
	 *
	 * Extends: Sequence<IfBranch, Branch>
	 *
	 * Exports:
	 *   _else_ - optional fall-through case
	 */
	struct IfElse : Sequence<IfBranch, Branch> {
		ptr<ValExpr> _else_;

		IfElse(std::deque<ptr<IfBranch>>, ptr<ValExpr>, Location);

		virtual void accept(Visitor&);
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
		ptr<TuplePattern> vars;
		ptr<ValExpr> expr;
		ptr<ValExpr> if_guard;

		Case(ptr<TuplePattern>, ptr<ValExpr>, ptr<ValExpr>, Location);

		virtual void accept(Visitor&);
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

		virtual void accept(Visitor&);
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
		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	//
	// STMTS
	//
	// This section represents all the compound statements otherwise not declared
	// 
	
	/*
	 * Represents a module declaration expression
	 *
	 * Extends: Statement
	 *
	 * Exports:
	 *   module - the name of the created module
	 */
	struct ModDec : Statement {
		ptr<QualifiedBinding> module;

		ModDec(ptr<QualifiedBinding>, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Represents an implements expression within a type body used to import functions
	 *   into the types interface. Almost entirely similar to standard OOP-inheritance
	 *
	 * Extends: Statement
	 *
	 * Exports:
	 *   type - the type that is being implemented
	 *   impls - block containing implementations for imported functions
	 *   _interface - type interface that is being implemented for the original type
	 */
	struct ImplExpr : Statement {
		ptr<SourceType> _interface;
		ptr<SourceType> type;
		ptr<Block> impls;

		ImplExpr(ptr<SourceType>, ptr<SourceType>, ptr<Block>, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Represents a rebind/import statement
	 *
	 * Extends: Statement
	 */
	struct ModRebindImport : Statement {
		ptr<QualifiedBinding> _module;

		ModRebindImport(Location);
		ModRebindImport(ptr<QualifiedBinding>, Location);

		virtual void accept(Visitor&) = 0;
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
	};

	/*
	 * Import a single binding into the current scope
	 *
	 * Extends: ModRebindImport
	 *
	 * Exports:
	 *   imp - the binding that is being imported
	 */
	struct SingleImport : ModRebindImport {
		ptr<BasicBinding> binding;

		SingleImport(ptr<QualifiedBinding>, ptr<BasicBinding>, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Import multiple bindings into the current scope
	 *
	 * Extends: Sequence<BasicBinding, ModRebindImport>
	 */
	struct MultipleImport : Sequence<BasicBinding, ModRebindImport> {
		MultipleImport(ptr<QualifiedBinding>, std::deque<ptr<BasicBinding>>, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Rebind a name in the current scope in some fashion
	 *
	 * Extends: ModRebindImport
	 *
	 * Exports:
	 */
	struct Rebind : ModRebindImport {
		ptr<BasicBinding> old_name;
		ptr<Array> old_gen;

		ptr<BasicBinding> new_name;
		ptr<Array> new_gen;

		Rebind(ptr<QualifiedBinding>, ptr<BasicBinding>, ptr<Array>, ptr<BasicBinding>, ptr<Array>, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

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
	struct Interface : Statement {
		VisibilityType vis;
		ptr<AssignPattern> name;
		ptr<GenericArray> gen;
		ptr<Type> type;

		Interface(VisibilityType, ptr<AssignPattern>, ptr<GenericArray>, ptr<Type>, Location);

		virtual void accept(Visitor&);
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
		using ConsList = std::deque<ptr<Constructor>>;
		ConsList cons;
		ptr<Block> body;
		bool mutable_only;

		TypeAssign(VisibilityType, ptr<AssignPattern>, ptr<GenericArray>, bool, ConsList, ptr<Block>, Location);

		virtual void accept(Visitor&);
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

		VarAssign(VisibilityType, ptr<AssignPattern>, ptr<GenericArray>, ptr<Type>, ptr<ValExpr>, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Represents an anonymous type extension
	 *
	 * Extends: Statement
	 *
	 * Exports:
	 *   typ_name - type being extended
	 *   gen - generic instance arguments for the extended type
	 *   args - arguments for the extended type's constructor
	 *   ext - extension block
	 */
	struct TypeExtension : ValExpr {
		ptr<QualifiedBinding> typ_name;
		ptr<Array> gen;
		ptr<Tuple> args;
		ptr<Block> ext;

		TypeExtension(ptr<QualifiedBinding>, ptr<Array>, ptr<Tuple>, ptr<Block>, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	//
	// EXPRESSIONS
	//
	// This section represents all the compound statements that produce values
	//

	/*
	 * Represents a scoped binding assignment, where a variable is declared for use in
	 *   a single expression and is only visible within that context
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   bind - the assignment binding the variable to some value
	 *    expr - the statement in which the binding is visible
	 */
	struct InAssign : ValExpr {
		ptr<VarAssign> bind;
		ptr<ValExpr> expr;

		InAssign(ptr<VarAssign>, ptr<ValExpr>, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Basic class that represents a variable name
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   name - qualified binding that represents the variable
	 *   inst_args - generic instantiation argument array
	 */
	struct Variable : ValExpr {
		ptr<QualifiedBinding> name;
		ptr<Array> inst_args;

		Variable(ptr<QualifiedBinding>, ptr<Array>, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Represents an unary operator call
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   expr - expression that the unary operator is applied to
	 *   op - operator that is being invoked
	 */
	struct UnOpCall : ValExpr {
		ptr<ValExpr> expr;
		ptr<BasicBinding> op;
		// std::string op;

		//UnOpCall(ptr<ValExpr>, std::string, Location);
		UnOpCall(ptr<ValExpr>, ptr<BasicBinding>, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

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
		std::string op;

		BinOpCall(ptr<ValExpr>, ptr<ValExpr>, std::string, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Represents a sequence of values where the each one is used to index the prior
	 *
	 * Extends: Sequence<ValExpr>
	 */
	struct Index : Sequence<ValExpr> {
		Index(std::deque<ptr<ValExpr>>, Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Represents a complete step of fuction sequencing
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   callee - expression that applies the function sequence
	 *   arguments   - argument tuple section
	 *   instance   - generic instantiation section
	 */
	struct FnCall : ValExpr {
		ptr<ValExpr> callee;
		ptr<Tuple> arguments;

		FnCall(ptr<ValExpr>, ptr<Tuple>, Location);


		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	
	//
	// ERRORS
	//
	// This section represents all structures that represent something wrong
	//

	/*
	 * Helper node for tracking a symbol appearance for error reporting
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   ch - the character that this node was created for
	 */
	struct Symbol : Ast {
		char ch;

		Symbol(char, Location);
	};

	/*
	 * Base node for representing all error contexts
	 *   Provides overload of `prettyPrint` for all error classes
	 *
	 * Extends: Ast
	 */
	struct Error : Ast {
		Error(Location loc);

		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};

	/*
	 * Sentinel struct representing a missing close symbol
	 *
	 * Extends: Error
	 */
	struct CloseSymbolError : Error {
		CloseSymbolError(Location loc) : Error{ loc } {}
	};

	/*template<class T>
	struct Error : T { };

	template<class T>
	struct SyntaxError : Error<T> {};*/

}
