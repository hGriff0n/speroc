#pragma once

#include <variant>

namespace spero::compiler::ast {
	// Literals
	struct Byte;
	struct Int;
	struct Float;
	struct String;
	struct Char;
	struct Bool;
	struct Tuple;

	// Bindings
	struct Var;

	// Ast Productions
	struct Sentinel {};
	enum class Keyword {
		MUT,
		LET,
		DEF,
		STATIC,
		USE,
		MOD,
		MATCH,
		IF,
		ELSIF,
		ELSE,
	};
}

namespace spero::compiler {
	using litnode = std::variant<ast::Byte, ast::Int, ast::Float, ast::String, ast::Char, ast::Bool, ast::Tuple>;
	using bindnode = std::variant<ast::Var>;
	using astnode = std::variant<litnode, ast::Sentinel, ast::Keyword>;
}