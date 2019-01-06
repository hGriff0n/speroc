#include "driver/CompilationDriver.h"

#pragma warning(push, 0)
#pragma warning(disable:4996)
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>
#pragma warning(pop)

using namespace spero;
using namespace spero::compiler;

CompilationDriver::CompilationDriver(CompilationState& state, analysis::AllTypes& types) : AnalysisDriver{ state, types } {}

#define ASM_COMPILER "clang"
#define ASM_TEMP_OUTPUT_FILE "out.ll"
#define TIMER(name) auto _ = state.timer(name)

void CompilationDriver::writeLlvmToFile() {
	if (!state.failed()) {
		TIMER("assembly_output");

		std::error_code ec;
		llvm::raw_fd_ostream out_stream{ ASM_TEMP_OUTPUT_FILE, ec, llvm::sys::fs::F_None };

		translation_unit->setDataLayout(state.targetDataLayout());
		translation_unit->setTargetTriple(state.targetTriple());
		translation_unit->print(out_stream, nullptr);

		out_stream.close();
	}
}

void CompilationDriver::triggerClangCompile() {
	if (!state.failed() && state.produceExe()) {
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

	// analysis
	analyzeAst();

	// backend
	translateAstToLlvm();
	optimizeLlvm();

	// compilation
	writeLlvmToFile();
	triggerClangCompile();

	return state.failed();

} catch (std::exception& e) {
	state.log(ID::err, e.what());
	return true;
}