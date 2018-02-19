#pragma once

#include <fstream>
#include <string>

#include "interface/CompilationState.h"
#include "util/asmjit.h"


namespace spero::compiler {
	// Perform all steps related to parsing and initial IR creation
	parser::Stack parse(std::string, CompilationState&);
	parser::Stack parseFile(std::string, CompilationState&);

	// Perform all steps related to analysis and secondary IR creation (may abstract IR to a different function)
	using MIR_t = parser::Stack;
	MIR_t analyze(parser::Stack, CompilationState&);
	
	// Perform all steps related to backend production
	gen::Assembler backend(MIR_t&, CompilationState&);

	// Perform all steps related to final codegen stages (produces assembly code)
	void codegen(gen::Assembler&, const std::string&, const std::string&, CompilationState&, bool=true);

	// Run a JIT interpretation environment
	void interpret(gen::Assembler&);
}


namespace spero {
	bool compile(compiler::CompilationState&, parser::Stack&);
}