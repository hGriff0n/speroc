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
	template<class T, class Inher = T>
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


	//
	// TYPES
	//
	// This section contains the nodes to collect information about types in source files
	//

	struct TType : Ast {};


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
		//ptr<Tuple> args;

		//Annotation(ptr<BasicBinding>, ptr<Tuple>, Location);
		Annotation(ptr<BasicBinding>, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");
	};


	/*
	* Represents an annotation instance that is attached to a statement
	*
	* Extends: LocalAnnotation
	*/
	struct LocalAnnotation : Annotation {
		//LocalAnnotation(ptr<BasicBinding>, ptr<Tuple>, Location);
		LocalAnnotation(ptr<BasicBinding>, Location);

		virtual Visitor& visit(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "") final;
	};


	//
	// CONTROL
	//
	// This section contains the nodes for all Spero control structures
	//

	struct Control : Ast {};


	//
	// STMTS
	//
	// This section represents all the compound statements in Spero not already presented
	//

	struct Stmt : Ast {};

}