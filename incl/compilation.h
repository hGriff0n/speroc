#pragma once

#include <fstream>
#include <string>

#include "interface/CompilationState.h"


namespace spero::compiler {
	// Perform all steps related to parsing and initial IR creation
	std::tuple<size_t, spero::parser::Stack> parse(std::string, CompilationState&);
	std::tuple<size_t, spero::parser::Stack> parseFile(std::string, CompilationState&);


	// Perform all steps related to analysis and secondary IR creation (may abstract IR to a different function)
	using IR_t = spero::parser::Stack;
	IR_t analyze(spero::parser::Stack, CompilationState&);

	
	// Perform all steps related to backend production
	gen::Assembler backend(IR_t&, CompilationState&);


	// Perform all steps related to final codegen stages (produces assembly code)
	void codegen(IR_t&, const std::string&, const std::string&, CompilationState&, bool=true);
}


namespace spero {
	bool compile(compiler::CompilationState&, parser::Stack&);
}