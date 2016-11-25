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
	struct can_hold : std::is_same<T, V> {};

	template<class T, class... V>
	struct can_hold<T, std::variant<V...>> : std::disjunction<std::is_same<T, V>...> {};
}

namespace spero::compiler::ast {
	/*
	 * Enums and other basic types
	 */
	using Sentinel = nullptr_t;
	BETTER_ENUM(KeywordType, char, LET, DEF, STATIC, MUT, DO,
				MOD, USE, MATCH, IF, ELSIF, ELSE, WHILE, FOR, LOOP,
				BREAK, CONT, YIELD, RET, WAIT, IMPL, F_IN);
	BETTER_ENUM(PtrStyling, char, POINTER, VIEW, NA);
	BETTER_ENUM(VarianceType, char, COVARIANT, CONTRAVARIANT, INVARIANT, NA);
	BETTER_ENUM(RelationType, char, IMPLS, NOT_IMPLS, SUBTYPE, SUPERTYPE, NA);
	BETTER_ENUM(VisibilityType, char, PUBLIC, PROTECTED, PRIVATE, STATIC);
	BETTER_ENUM(BindingType, char, TYPE, VARIABLE, OPERATOR);
	BETTER_ENUM(UnaryType, char, DEREF, NOT, MINUS, NA);
	struct Annotation;
	struct Type;


	/*
	 * Base class for all ast nodes
	 *
	 * Exports:
	 *   pretty_print - function to control pretty printing of ast nodes
	 */
	struct Ast {
		Ast();

		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "");
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
		using token_type = std::variant<KeywordType, PtrStyling, VarianceType, RelationType, VisibilityType, BindingType, UnaryType>;
		token_type value;

		//template<class T, class = std::enable_if_t<can_hold<T, token_type>::value>>
		//Token(const T val) : value{ val } {}
		Token(KeywordType);
		Token(PtrStyling);
		Token(VarianceType);
		Token(RelationType);
		Token(VisibilityType);
		Token(BindingType);
		Token(UnaryType);

		OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
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

		Stmt();
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
		UnaryType unop = UnaryType::NA;
		ptr<Type> type;

		ValExpr();
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "");
	};
}

namespace spero::parser {
	using Stack = std::deque<compiler::ptr<compiler::ast::Ast>>;
}