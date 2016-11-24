#pragma once

#include "node_defs.h"
#include <variant>

#include <iostream>

namespace spero::compiler::ast {
	// Bring in dependency on iostream (TODO: Find a better way to handle this)
	using IoStream = std::ostream&;

	/*
	 * Templates to check if a variant could hold a T
	 */
	template<class T, class V>
	struct can_hold : std::is_same<T, V> {};

	template<class T, class... V>
	struct can_hold<T, std::variant<V...>> : std::disjunction<std::is_same<T, V>...> {};



	/*
	 * Base class for all ast nodes
	 *
	 * Exports:
	 *   pretty_print - function to control pretty printing of ast nodes
	 */
	struct Ast {
		Ast();

		virtual IoStream prettyPrint(IoStream, size_t = 0);
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

		template<class T, class = std::enable_if_t<can_hold<T, token_type>::value>>
		Token(T val) : value{ val } {}

		IoStream prettyPrint(IoStream, size_t = 0) final;
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
		virtual IoStream prettyPrint(IoStream, size_t = 0);
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
		virtual IoStream prettyPrint(IoStream, size_t = 0);
	};
}