#pragma once

#include <variant>
#include <memory>
#include <deque>
#include <string>
#include "enum.h"

#include <iostream>

namespace spero::compiler {
	template<class T>
	using ptr = std::unique_ptr<T>;


	// Bring in dependency on iostream (TODO: Find a better way to handle this)
	using OutStream = std::ostream;


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
				BREAK, CONT, YIELD, RET, WAIT, IMPL, F_IN);
	BETTER_ENUM(PtrStyling, char, PTR, REF, VIEW, NA);
	BETTER_ENUM(VarianceType, char, COVARIANT, CONTRAVARIANT, INVARIANT, VARIADIC);
	BETTER_ENUM(RelationType, char, IMPLS, NOT_IMPLS, SUBTYPE, SUPERTYPE, NA);
	BETTER_ENUM(VisibilityType, char, PUBLIC, PROTECTED, PRIVATE, STATIC);
	BETTER_ENUM(BindingType, char, TYPE, VARIABLE, OPERATOR);
	BETTER_ENUM(UnaryType, char, DEREF, NOT, MINUS, RESERVED, NA);
	BETTER_ENUM(CaptureType, char, NORM, MUT, REF, MUTREF);
	struct Annotation;


	/*
	 * Base class for all ast nodes
	 *
	 * TODO: Figure out what needs to go in the location structure
	 *
	 * Exports:
	 *   pretty_print - function to control pretty printing of ast nodes
	 *   loc - structure that holds the location data for the node
	 */
	struct Ast {
		struct Location {
			size_t byte, line;
			std::string src;
		} loc;

		Ast(Ast::Location);

		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "");
		virtual OutStream& assemblyCode(OutStream&);
	};


	/*
	 * Basic class for representing one of the token nodes
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   value - a variant of all token types
	 */
	struct Token : Ast {
		using token_type = std::variant<KeywordType, PtrStyling, VarianceType, RelationType,
										VisibilityType, BindingType, UnaryType, CaptureType>;
		token_type value;

		template<class T, class = std::enable_if_t<can_hold_v<T, token_type>>>
		Token(T val, Ast::Location loc) : Ast{ loc }, value { val } {}

		OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	* Base class for all type nodes
	*
	* Extends: Ast
	*   Reuses `prettyPrint` definition
	*
	* Exports:
	*   id - type id, assigned during analysis
	*   is_mut - flag whether values of the type are  mutable
	*
	* TODO:
	*   Need to have a better handle on type id assignment and usage across the ast in the future
	*/
	struct Type : Ast {
		size_t id;
		bool is_mut = false;

		Type(Ast::Location);
	};


	/*
	 * Basic class to represent a Spero statement
	 *   TODO: Differentiate from ValExpr
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   annotations - A collection of associated annotations
	 */
	struct Stmt : Ast {
		std::deque<ptr<Annotation>> annots;

		Stmt(Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "");
	};


	/*
	 * Basic class to handle Spero statements that produce a value
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
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "");
	};


	/*
	 * Simple sentinel for handling anon_type failure
	 *
	 * [deprecated] - meant to be used to resolve anon_type - type_tuple clases
	 */
	//struct AnonTypeSeperator : Ast {};

}

namespace spero::parser {
	using Stack = std::deque<compiler::ptr<compiler::ast::Ast>>;
}