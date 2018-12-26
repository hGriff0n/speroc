#pragma once

#include <fstream>
#include <string>

#pragma warning(push, 0)
#pragma warning(disable:4996)
#include <llvm/IR/Module.h>
#pragma warning(pop)

#include "interface/CompilationState.h"
#include "analysis/AnalysisState.h"

#define WITH(x) if (auto _ = (x); true)

// Because AsmJit somehow decides my computers architecture is 32-bit (x86) and I've currently hardcoded assembly generation,
// Since my installation of clang is 64-bit (x86-64), we have to force compilation under 32-bit mode
#define ASM_COMPILER "clang -masm=intel -m32"
#define ASM_TEMP_OUTPUT_FILE "out.ll"


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
	// TODO: `will_be_interpreted` is a temporary solution for repl compilation (requires everything in a function atm)
	llvm::Module* backend(MIR_t globals, parser::Stack& ast_stack, CompilationState& state, bool using_repl);

	// Perform all steps related to final codegen stages (produces assembly code)
	void codegen(llvm::Module* ir, const std::string& input_file, const std::string& output_file, CompilationState& state);

	// Run a JIT interpretation environment
	void interpret(llvm::Module* llvm_code, CompilationState& state);
}


namespace spero {
	bool compile(compiler::CompilationState& state, parser::Stack& ast_stack);

	// TODO: This will likely be subsumed once I get around to fixing the logging system
	template<class Fn>
	bool compile(compiler::CompilationState& state, parser::Stack& ast_stack, Fn&& irHook) try {
		using namespace spero;
		using parser::ParsingMode;

		const GET_PERMISSIONS(state);
		WITH(state.timer("parsing")) {
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
		}

		if (!do_compile) {
			return true;
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
		 * Run through the backend optimizations
		 *
		 * Don't mark this as a separate "phase" for timing purposes
		 * The sub-phases perform their own timing passes
		 */
		auto llvm_code = (!state.failed())
			? compiler::backend(std::move(table), ast_stack, state, parser_mode == ParsingMode::STRING)
			: new llvm::Module("speroc_error_state", state.getContext());

		/*
		 * Generate the boundary ir for the external tools
		 *
		 * speroc does not handle the generation of executables
		 * and other binary files, prefering to pass those stages
		 * off to some system tool that is guaranteed to work
		 */
		if (link && !state.failed()) {
			auto _ = state.timer("codegen");
			compiler::codegen(llvm_code, state.files()[0], ASM_TEMP_OUTPUT_FILE, state);
		}

		irHook(llvm_code);


		/*
		 * Send the boundary ir off to the final compilation phase
		 */
		if (!state.failed() && state.produceExe() && link) {
			if (auto _ = state.timer("assembly");
				system((ASM_COMPILER " " ASM_TEMP_OUTPUT_FILE " -o " + state.output()).c_str()))
			{
				state.log(compiler::ID::err, "Compilation of `{}` failed", state.output());
			}

			// Delete the temporary file
			if (state.deleteTemporaryFiles()) {
				std::remove(ASM_TEMP_OUTPUT_FILE);
			}
		}

		if (!state.failed() && interpret) {
			compiler::interpret(llvm_code, state);
		}

		return state.failed();

	} catch (std::exception& e) {
		state.log(compiler::ID::err, e.what());
		return false;
	}
}