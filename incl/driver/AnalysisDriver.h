#pragma once

#include "interface/CompilationState.h"
#include "analysis/AnalysisState.h"

namespace llvm {
	class Module;
	class LLVMContext;
}

namespace spero::compiler {

	using SperoModule = std::unique_ptr<llvm::Module>;

	class AnalysisDriver {
	protected:
		CompilationState& state;
		llvm::LLVMContext& context;

		parser::Stack ast;
		analysis::AnalysisState decls;
		SperoModule translation_unit = nullptr;

	protected:
		// Frontend: source -> AST
		void parseInput(parser::ParsingMode parser_mode, const std::string& input);

		// Analysis: AST -> MIR (LLVM IR)
		void analyzeAst();

		// Backend: MIR (LLVM IR) -> LLVM IR
		void translateAstToLlvm();
		void optimizeLlvm();

	public:
		AnalysisDriver(CompilationState& state, analysis::AllTypes& types);

		inline CompilationState& getState() {
			return state;
		}
	};

}