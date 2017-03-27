#pragma once

#include <fstream>
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

namespace spero::parser {
	using Stack = std::deque<compiler::ptr<compiler::ast::Ast>>;

	// Helper function to print out the ast
	std::ostream& printAST(std::ostream&, const Stack&);
}


namespace spero::compiler {
	// Perform all steps related to parsing and initial IR creation
	std::tuple<bool, spero::parser::Stack> parse(std::string);
	std::tuple<bool, spero::parser::Stack> parseFile(std::string);

	// Perform all steps related to analysis and secondary IR creation (may abstract IR to a differnt function)
	using IR_t = spero::parser::Stack;
	IR_t analyze(spero::parser::Stack&);

	// Perform all steps related to final codegen stages (produces assembly code)
	void codegen(IR_t&, std::string, std::string, std::ostream&);
}


namespace spero {
	bool compile(parser::Stack&, std::string, std::string);
}