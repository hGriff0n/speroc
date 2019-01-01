#include "driver/CompilationDriver.h"

#pragma warning(push, 0)
#pragma warning(disable:4996)
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>
#pragma warning(pop)

using namespace spero;
using namespace spero::compiler;

CompilationDriver::CompilationDriver(CompilationState& state, analysis::AllTypes& types) : AnalysisDriver{ state, types } {}

#define TIMER(name) auto _ = state.timer(name)
void CompilationDriver::writeLlvmToFile(bool perform_compilation) {
	if (!state.failed() && perform_compilation) {
		TIMER("assembly_output");

		std::error_code ec;
		llvm::raw_fd_ostream out_stream{ state.output(), ec, llvm::sys::fs::F_None };

		translation_unit->setDataLayout(state.targetDataLayout());
		translation_unit->setTargetTriple(state.targetTriple());
		translation_unit->print(out_stream, nullptr);
	}
}

#define ASM_COMPILER "clang"
#define ASM_TEMP_OUTPUT_FILE "out.ll"
void CompilationDriver::triggerClangCompile(bool perform_compilation) {
	if (!state.failed() && state.produceExe() && perform_compilation) {
		TIMER("clang_compilation");

		auto clang_command = ASM_COMPILER " " ASM_TEMP_OUTPUT_FILE " -o " + state.output();
		auto failure = system(clang_command.c_str());
		if (failure) {
			state.log(ID::err, "Clang compilation to `{}` failed", state.output());
		}
		
		if (state.deleteTemporaryFiles()) {
			std::remove(ASM_TEMP_OUTPUT_FILE);
		}
	}
}

bool CompilationDriver::compile() try {
	const GET_PERMISSIONS(state);

	// frontend
	parseInput(parser_mode, state.files()[0]);

	if (do_compile) {
		// analysis
		analyzeAst();

		// backend
		translateAstToLlvm();
		optimizeLlvm();

		// compilation
		writeLlvmToFile(link);
		triggerClangCompile(link);
	}

	return state.failed();

} catch (std::exception& e) {
	state.log(ID::err, e.what());
	return true;
}