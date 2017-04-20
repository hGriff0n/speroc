#pragma once

#include <fstream>
#include <string>

#include "util\CompilationState.h"


namespace spero::compiler {

	// Perform all steps related to parsing and initial IR creation
	std::tuple<bool, spero::parser::Stack> parse(std::string, CompilationState&);
	std::tuple<bool, spero::parser::Stack> parseFile(std::string, CompilationState&);

	// Perform all steps related to analysis and secondary IR creation (may abstract IR to a differnt function)
	using IR_t = spero::parser::Stack;
	IR_t analyze(spero::parser::Stack&, CompilationState&);

	// Perform all steps related to final codegen stages (produces assembly code)
	void codegen(IR_t&, std::string, std::string, CompilationState&);
}


namespace spero {
	bool compile(compiler::CompilationState&, parser::Stack&, std::string);
}