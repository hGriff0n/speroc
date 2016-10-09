#pragma once

#include <variant>
#include <optional>
#include <string>

namespace spero::compiler::ast {
	struct ast {};

	// Literals
	struct Byte {
		int val;
		Byte(const std::string&, int base);
	};
	struct Int {
		int val;
		Int(const std::string&);
	};
	struct Float {
		float val;
		Float(const std::string&);
	};
	struct String {
		std::string val;
		String(const std::string&);
	};
	struct Char {
		char val;
		Char(char);
	};
}

namespace spero::compiler {
	using litnode = std::variant<ast::Byte, ast::Int, ast::Float, ast::String, ast::Char>;
	using astnode = std::variant<litnode>;
}