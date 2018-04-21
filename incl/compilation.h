#pragma once

#include <fstream>
#include <string>

#include "interface/CompilationState.h"
#include "util/asmjit.h"


namespace spero::compiler {
	// Perform all steps related to parsing and initial IR creation
	parser::Stack parse(std::string input, CompilationState& state);
	parser::Stack parseFile(std::string file, CompilationState& state);

	// Perform all steps related to analysis and secondary IR creation (may abstract IR to a different function)
	// NOTE: I don't have a second IR at the moment, so this stage just serves to prepare everything for the backend
	using MIR_t = std::unique_ptr<analysis::SymTable>;
	MIR_t analyze(parser::Stack& ast_stack, CompilationState& state);
	
	// Perform all steps related to backend production
	gen::Assembler backend(MIR_t globals, parser::Stack& ast_stack, CompilationState& state);

	// Perform all steps related to final codegen stages (produces assembly code)
	void codegen(gen::Assembler& emit, const std::string& input_file, const std::string& output_file, CompilationState& state, bool output_header=true);

	// Run a JIT interpretation environment
	void interpret(gen::Assembler& asm_code);
}


namespace spero {
	bool compile(compiler::CompilationState& state, parser::Stack& ast_stack);
}