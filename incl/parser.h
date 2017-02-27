#pragma once

#include <memory>
#include <string>
#include <deque>


// Forward Declarations
namespace spero::compiler {
	template<class T> using ptr = std::unique_ptr<T>;
}

namespace spero::compiler::ast {
	struct Ast;
}


// Parser interface for the rest of the compiler
namespace spero::parser {
	using Stack = std::deque<compiler::ptr<compiler::ast::Ast>>;

	std::tuple<bool, Stack> parse(std::string);
	std::tuple<bool, Stack> parseFile(std::string);
	//parse_file
}