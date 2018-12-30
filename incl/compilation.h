#pragma once

#include <fstream>
#include <string>

#pragma warning(push, 0)
#pragma warning(disable:4996)
#include <llvm/IR/Module.h>
#include <ExecutionEngine/Interpreter/Interpreter.h>
#pragma warning(pop)

#include "interface/CompilationState.h"
#include "analysis/AnalysisState.h"

#define WITH(x) if (auto _ = (x); true)

// Because AsmJit somehow decides my computers architecture is 32-bit (x86) and I've currently hardcoded assembly generation,
// Since my installation of clang is 64-bit (x86-64), we have to force compilation under 32-bit mode
#define ASM_COMPILER "clang -masm=intel -m32"
#define ASM_TEMP_OUTPUT_FILE "out.ll"


// TODO: A restructuring of these interfaces is in order sometime in the near future
// However, I'm still uncertain of how the new interfaces will look
namespace spero::compiler {
	// Structure for specifying what stages should be run
	using Permissions = std::tuple<int, int, bool, bool>;

	// Perform all steps related to parsing and initial IR creation
	parser::Stack parse(std::string input, CompilationState& state);
	parser::Stack parseFile(std::string file, CompilationState& state);

	// Perform all steps related to analysis and secondary IR creation (may abstract IR to a different function)
	// NOTE: I don't have a second IR at the moment, so this stage just serves to prepare everything for the backend
	using MIR_t = decltype(std::declval<analysis::AnalysisState>().arena);
	MIR_t analyze(parser::Stack& ast_stack, CompilationState& state, analysis::AllTypes&);
	
	// Perform all steps related to backend production
	void optimizeLlvmIr(std::unique_ptr<llvm::Module>& ir, CompilationState& state);
	std::unique_ptr<llvm::Module> backend(MIR_t globals, parser::Stack& ast_stack, CompilationState& state);

	// Perform all steps related to final codegen stages (produces assembly code)
	void codegen(std::unique_ptr<llvm::Module>& ir, const std::string& input_file, const std::string& output_file, CompilationState& state);

	// Run a JIT interpretation environment
	void interpret(llvm::Interpreter* inter, std::unique_ptr<llvm::Module> llvm_code, CompilationState& state);
	void transformAstForInterpretation(parser::Stack& ast_stack, CompilationState& state);
}


namespace spero {
	bool compile(compiler::CompilationState& state, parser::Stack& ast_stack);

	// TODO: This will likely be subsumed once I get around to fixing the logging system
	template<class Fn>
	bool compile(compiler::CompilationState& state, parser::Stack& ast_stack, llvm::Interpreter* inter, Fn&& irHook) try {
		using namespace spero;
		using parser::ParsingMode;

		const GET_PERMISSIONS(state);
		switch (parser_mode) {
			case ParsingMode::FILE:
				ast_stack = compiler::parseFile(state.files()[0], state);
				break;
			case ParsingMode::STRING:
				ast_stack = compiler::parse(state.files()[0], state);
				break;
			default:
				return true;
		}

		if (!do_compile) {
			irHook(ast_stack);
			return true;
		}

		// In order to run/show code in the repl, we have to store everything inside of a function
		// We currently, perform this wrapping in `codegen` but it may be beneficial to instead do it here
		auto using_repl = inter != nullptr;
		if (using_repl) {
			transformAstForInterpretation(ast_stack, state);
		}

		/*
		 * Run through the analysis stages
		 *
		 * Don't mark this as a separate "phase" for timing purposes
		 * The sub-phases perform their own timing passes
		 */
		auto type_list = analysis::initTypeList();
		auto table = (!state.failed())
			? compiler::analyze(ast_stack, state, type_list)
			: analysis::SymArena{};

		irHook(ast_stack);


		/*
		 * Translate the analysed spero code into llvm ir
		 *
		 * NOTE: Timing is handled inside of the function
		 */
		auto llvm_code = (!state.failed())
			? compiler::backend(std::move(table), ast_stack, state)
			: std::make_unique<llvm::Module>("speroc_error_state", state.getContext());

		irHook(llvm_code);
		
		/*
		 * Optimize the llvm ir code
		 */
		bool do_optimize = false;
		if (do_optimize && !state.failed()) {
			compiler::optimizeLlvmIr(llvm_code, state);
		}

		/*
		 * Generate the boundary ir for the external tools
		 *
		 * speroc does not handle the generation of executables
		 * and other binary files, prefering to pass those stages
		 * off to some system tool that is guaranteed to work
		 */
		if (link && !state.failed()) {
			compiler::codegen(llvm_code, state.files()[0], ASM_TEMP_OUTPUT_FILE, state);
		}


		/*
		 * Send the boundary ir off to the final compilation phase
		 */
		if (!state.failed() && state.produceExe() && link) {
			auto _ = state.timer("clang_compilation");
			if (system((ASM_COMPILER " " ASM_TEMP_OUTPUT_FILE " -o " + state.output()).c_str())) {
				state.log(compiler::ID::err, "Compilation of `{}` failed", state.output());
			}

			// Delete the temporary file
			if (state.deleteTemporaryFiles()) {
				std::remove(ASM_TEMP_OUTPUT_FILE);
			}
		}

		// Run the interpreter on the final llvm code if requested
		if (!state.failed() && interpret && using_repl) {
			compiler::interpret(inter, std::move(llvm_code), state);
		}

		return state.failed();

	} catch (std::exception& e) {
		state.log(compiler::ID::err, e.what());
		return false;
	}
}