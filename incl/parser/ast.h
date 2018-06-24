#pragma once

#include <variant>
#include <deque>

#include <enum.h>

#include "analysis/SymTable.h"
#include "analysis/types.h"

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
		BREAK, CONT, YIELD, RET, WAIT, IMPL, F_IN, AS)
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
		using token_type = std::variant<KeywordType, PtrStyling, VarianceType, RelationType, VisibilityType, BindingType, CaptureType>;
		token_type value;

		template<class T, class = std::enable_if_t<can_hold_v<T, token_type>>>
		Token(T val, Location loc) : Ast{ loc }, value{ val } {}

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "");

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

		Type(Location loc);
		virtual void accept(AstVisitor& v);
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

		Statement(Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "");
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
		std::shared_ptr<analysis::Type> type = nullptr;

		ValExpr(Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "");
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

		using iterator = typename std::deque<ptr<T>>::iterator;

		Sequence(std::deque<ptr<T>> e, Location loc) : Inher{ loc }, elems{ std::move(e) } {}

		virtual void accept(AstVisitor& v) =0;
	};



	//
	// LITERALS, ATOMS
	//
	// This section contains the nodes representing value-types that are compile-time recognizable
	//

	struct Literal : ValExpr {
		Literal(Location loc);
	};

	struct Bool : Literal {
		bool val;
		Bool(bool val, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	struct Byte : Literal {
		unsigned long val;
		Byte(const std::string& val, int base, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	struct Float : Literal {
		double val;
		Float(const std::string& num, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	struct Int : Literal {
		long val;
		Int(const std::string& num, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	struct Char : Literal {
		char val;
		Char(char c, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	// NOTE: Do not intern these strings! These are user-produced string literals, not variables/etc.
	struct String : ValExpr {
		std::string val;
		String(std::string str, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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

		Future(bool is_comp_generated, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	/*
	 * Represents a statically-sized indexable grouping of values with different types
	 *
	 * Extends: Sequence<ValExpr>
	 */
	struct Tuple : Sequence<ValExpr> {
		Tuple(std::deque<ptr<ValExpr>> vals, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	/*
	 * Represents a dynamically-sized indexable grouping of values sharing the same base type
	 *
	 * Extends: Sequence<ValExpr>
	 */
	struct Array : Sequence<ValExpr> {
		Array(std::deque<ptr<ValExpr>> vals, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};


	/*
	 * Represents an ordered sequence of executable expressions
	 *   NOTE: The ordering property may depend on context (ie. in regards to types)
	 *
	 * Extends: Sequence<Statement, ValExpr>
	 *
	 * Exports:
	 *   locals - a symbol table collecting analysed information about locally declared variables
	 */
	struct Block : Sequence<Statement, ValExpr> {
		analysis::SymTable locals;

		Block(std::deque<ptr<Statement>> vals, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "");
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
	 *
	 * TODO: How are arguments supposed to work with/without a scope body
	 */
	struct Function : ValExpr {
		ptr<Tuple> args;
		ptr<ValExpr> body;
		std::optional<spero::String> name;

		Function(ptr<Tuple> args, ptr<ValExpr> body, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "");
	};


	// 
	// NAMES, BINDINGS, PATTERNS
	// 
	// This section contains the nodes representing bindings, names, and patterns
	// 

	/*
	 * Represents the basic block of a Spero binding. Basically just a string
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
		spero::String name;
		BindingType type;

		BasicBinding(spero::String str, BindingType type, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	/*
	 * A piece of a fully qualified spero binding name
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   name - the string name that this section maps to
	 *   type - whether this part is a type, variable, or operator
	 *   gens - array that is being used as the instantiation arguments
	 */
	struct PathPart : Ast {
		spero::String name;
		std::optional<size_t> ssa_id = std::nullopt;
		BindingType type;
		ptr<Array> gens;

		PathPart(spero::String str, BindingType type, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "");
	};

	template<class Stream>
	inline Stream& operator<<(Stream& s, const PathPart& part) {
		s << part.name;
		return s;
	}

	/*
	 * A fully qualified spero binding. This solution encapsulates the following cases
	 *   1) Basic bindings (ie. "foo" or "Bar")
	 *   2) Qualified bindings (ie. "std:foo")
	 *   3) Static type bindings (ie. "std:String:size")
	 *   4) Generic bindings (ie. "std:Vector[Int]:size")
	 *
	 * Extends: Sequence<PathPart, Ast>
	 */
	struct Path : Sequence<PathPart, Ast> {
		Path(ptr<PathPart> fst, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	template<class Stream>
	inline Stream& operator<<(Stream& s, const Path& loc) {
		s << *loc.elems.front();
		for (auto p = std::begin(loc.elems) + 1; p != std::end(loc.elems); ++p) {
			s << ':' << **p;
		}
		return s;
	}

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

		Pattern(Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "");
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
		TuplePattern(std::deque<ptr<Pattern>> parts, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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
		ptr<Path> name;

		VarPattern(ptr<Path> var, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "");
	};

	/*
	 * Instance class for matching against a decomposed ADT
	 *
	 * Extends: PNamed
	 *   NOTE: Uses name as the ADT type to match against
	 *
	 * Exports:
	 *   args - the tuple to match values against
	 */
	struct AdtPattern : VarPattern {
		ptr<TuplePattern> args;

		AdtPattern(ptr<Path> adt_name, ptr<TuplePattern> extract_pattern, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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

		ValPattern(ptr<ValExpr> value, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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
		AssignPattern(Location loc);

		// TODO: Look at replacing this with a 'VarData' structure
		// That or come up with a better way to pass data at assignment
		bool is_mut = false;

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "");
	};

	/*
	 * Represents a singular named binding in an assignment context
	 *
	 * Extends: AssignPattern
	 *
	 * Exports:
	 *   var - binding to assign to
	 */
	struct AssignName : AssignPattern {
		ptr<BasicBinding> var;

		AssignName(ptr<BasicBinding> name, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	/*
	 * Represents a decomposed tuple assignment
	 *
	 * Extends: AssignPattern
	 *
	 * Exports:
	 *   elems - a collection of AssignPattern to match against
	 */
	struct AssignTuple : Sequence<AssignPattern> {
		AssignTuple(std::deque<ptr<AssignPattern>> patterns, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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
		ptr<Path> name;
		PtrStyling _ptr;

		SourceType(ptr<Path> binding, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "");
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

		GenericType(ptr<Path> binding, ptr<Array> gen_args, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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
		TupleType(std::deque<ptr<Type>> types, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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

		FunctionType(ptr<TupleType> arg_types, ptr<Type> ret_type, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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
		AndType(ptr<Type> typs, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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
		OrType(ptr<Type> typs, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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

		Annotation(ptr<BasicBinding> annot_name, ptr<Tuple> args, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "");
	};

	/*
	 * Represents an annotation instance that is attached to a statement
	 *
	 * Extends: LocalAnnotation
	 */
	struct LocalAnnotation : Annotation {
		LocalAnnotation(ptr<BasicBinding> annot_name, ptr<Tuple> args, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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

		GenericPart(ptr<BasicBinding> binding, ptr<Type> typ, RelationType relation, Location loc);
		virtual void accept(AstVisitor& v);
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
		ptr<Type> default_type;

		VarianceType variance;
		bool variadic;

		TypeGeneric(ptr<BasicBinding> symbol, ptr<Type> def, Location loc);
		TypeGeneric(ptr<BasicBinding> symbol, ptr<Type> typ, RelationType relation, VarianceType variance_kind, bool is_variadic, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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
		ptr<ValExpr> default_val;

		ValueGeneric(ptr<BasicBinding> symbol, ptr<ValExpr> def, Location loc);
		ValueGeneric(ptr<BasicBinding> symbol, ptr<Type> typ, RelationType relation, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	/*
	 * Instance class for literal values being used in generic contexts
	 *
	 * Extends: GenericPart
	 *
	 * Exports:
 	 *   value - the literal value
     */
	struct LitGeneric : GenericPart {
		ptr<ValExpr> value;

		LitGeneric(ptr<ValExpr> val, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	/*
	 * Collection class for generic declaration information
	 *
	 * Extends: Sequence<GenericPart, Ast>
	 */
	struct GenericArray : Sequence<GenericPart, Ast> {
		GenericArray(std::deque<ptr<GenericPart>> elems, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	/*
	 * Ast subtype to annotate a type constructor
	 *   TODO: Currently empty (ie. see `Adt`, `ArgTuple`)
	 *
	 * Extends: Ast
	 */
	struct Constructor : Ast {
		Constructor(Location loc);

		virtual void accept(AstVisitor& v) = 0;
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

		Adt(ptr<BasicBinding> type_name, ptr<TupleType> args, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	/*
	 * Represents a statically added type annotation
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   type - static type annotation
	 */
	struct TypeAnnotation : ValExpr {
		ptr<ValExpr> expression;
		ptr<Type> typ;

		TypeAnnotation(ptr<ValExpr> val, ptr<Type> typ, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	/*
	 * Represents a function/constructor named argument (with type information)
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
		ptr<TypeAnnotation> var;

		Argument(ptr<BasicBinding> binding, ptr<Type> type, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	/*
	 * Represents a possible argument tuple for resolution in type construction
	 *
	 * Extends: Constructor
	 */
	struct ArgTuple : Sequence<Argument, Constructor> {
		ArgTuple(std::deque<ptr<Argument>> args, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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
		Branch(Location loc);

		virtual void accept(AstVisitor& v);
	};

	/*
	 * An infinite loop that just repeats the body forever
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   body - the body of the loop
	 */
	struct Loop : Branch {
		ptr<ValExpr> body;

		Loop(ptr<ValExpr> body, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "");
	};

	/*
	 * Represents a basic while loop construct, loop until test is true
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

		While(ptr<ValExpr> test, ptr<ValExpr> body, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	/*
	 * Represents a for loop construct, loop over the provided generator
	 *   TODO: Still need to determine how this is "implemented"
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

		For(ptr<Pattern> pat, ptr<ValExpr> gen, ptr<ValExpr> body, Location loc);

		virtual void accept(AstVisitor& v) final;
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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

		IfBranch(ptr<ValExpr> test, ptr<ValExpr> body, bool is_elsif, Location loc);

		virtual void accept(AstVisitor& v) final;
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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

		IfElse(std::deque<ptr<IfBranch>> ifs, ptr<ValExpr> _else, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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

		Case(ptr<TuplePattern> extraction_vars, ptr<ValExpr> guard, ptr<ValExpr> expr, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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

		Match(ptr<ValExpr> test, std::deque<ptr<Case>> cases, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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

		Jump(KeywordType kind, ptr<ValExpr> expr, Location loc);
		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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
	 *   _module - the name of the created module
	 */
	struct ModDec : Statement {
		ptr<Path> _module;

		ModDec(ptr<Path> path, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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

		ImplExpr(ptr<SourceType> import_type, ptr<SourceType> impl_type, ptr<Block> impls_body, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	/*
	 * Represents a rebind/import statement
	 *
	 * Extends: Statement
	 */
	struct ModRebindImport : Statement {
		ptr<Path> _module;

		ModRebindImport(Location loc);
		ModRebindImport(ptr<Path> mod, Location loc);

		virtual void accept(AstVisitor& v) = 0;
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "");
	};

	/*
	 * Import a single binding into the current scope
	 *
	 * Extends: ModRebindImport
	 */
	struct SingleImport : ModRebindImport {
		SingleImport(ptr<Path> mod, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	/*
	 * Import multiple bindings into the current scope
	 *
	 * Extends: Sequence<BasicBinding, ModRebindImport>
	 */
	struct MultipleImport : Sequence<PathPart, ModRebindImport> {
		MultipleImport(ptr<Path> mod, std::deque<ptr<PathPart>> names, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	/*
	 * Rebind a name in the current scope in some fashion
	 *
	 * Extends: ModRebindImport
	 * 
	 * Exports:
	 *   new_name - the new name for the imported entity
	 */
	struct Rebind : ModRebindImport {
		ptr<Path> new_name;

		Rebind(ptr<Path> mod, ptr<Path> new_binding, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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

		Interface(VisibilityType vis, ptr<AssignPattern> binding, ptr<GenericArray> gen_pattern, ptr<Type> type_bound, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "");
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

		TypeAssign(VisibilityType vis, ptr<AssignPattern> binding, ptr<GenericArray> gen_pattern, bool mut_only, ConsList constructors, ptr<Block> def, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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

		VarAssign(VisibilityType vis, ptr<AssignPattern> binding, ptr<GenericArray> gen_pattern, ptr<Type> type_bound, ptr<ValExpr> value, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	/*
	 * Represents an anonymous type extension
	 *
	 * Extends: Statement
	 *
	 * Exports:
	 *   typ_name - type being extended
	 *   args - arguments for the extended type's constructor
	 *   ext - extension block
	 */
	struct TypeExtension : ValExpr {
		ptr<Path> typ_name;
		ptr<Tuple> args;
		ptr<Block> ext;

		TypeExtension(ptr<Path> base_type, ptr<Tuple> base_args, ptr<Block> ext_def, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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
	 * TODO: Implement a pass that transforms this node into a function call/def combo
	 *   Can't completely replace due to construction of `asgn_val` rule
	 *   Don't replace until functions (particularly local functions) are stable
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
		analysis::SymTable binding;

		InAssign(ptr<VarAssign> binding, ptr<ValExpr> expr, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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
		ptr<Path> name;

		Variable(ptr<Path> symbol, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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

		//UnOpCall(ptr<ValExpr>, std::string, Location loc);
		UnOpCall(ptr<ValExpr> expr, ptr<BasicBinding> op, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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
		spero::String op;

		BinOpCall(ptr<ValExpr> lhs, ptr<ValExpr> rhs, spero::String op, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	/*
	 * Represents a sequence of values where the each one is used to index the prior
	 *
	 * Extends: Sequence<ValExpr>
	 */
	struct Index : Sequence<ValExpr> {
		Index(std::deque<ptr<ValExpr>> indices, Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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

		FnCall(ptr<ValExpr> calling_fn, ptr<Tuple> fn_args, Location loc);


		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
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

		Symbol(char c, Location loc);
	};

	/*
	 * Base node for representing some error contexts
	 *   Provides overload of `prettyPrint` for all error classes
	 *
	 * Extends: Ast
	 */
	struct Error : Ast {
		Error(Location loc);

		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	/*
	 * Sentinel struct representing a missing close symbol
	 *
	 * Extends: Error
	 */
	struct CloseSymbolError : Error {
		CloseSymbolError(Location loc);
	};

	/*
	 * Sentinel structs to allow for displaying "errors" in the context of other expected node types
	 */
	struct ValError : ValExpr {
		ValError(Location loc);

		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	struct TypeError : SourceType {
		TypeError(Location loc);

		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};

	struct ScopeError : Block {
		ScopeError(Location loc);

		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "") final;
	};
}

namespace spero::analysis {

	// If lookup succeeds, then the returned iterator is equal to `std::end(var_path.elems)`
	std::tuple<opt_t<SymTable::DataType>, compiler::ast::Path::iterator> lookup(SymTable& globals, SymTable* current, compiler::ast::Path& var_path);
	bool testSsaLookupFailure(opt_t<SymTable::DataType>& lookup_result, compiler::ast::Path::iterator& iter);

}