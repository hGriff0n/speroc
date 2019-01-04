#pragma once

#include "interface/CompilationState.h"
#include "analysis/AnalysisState.h"

namespace llvm {
	class LLVMContext;
	class Module;
}

namespace spero::compiler {

	using SperoModule = std::unique_ptr<llvm::Module>;
	struct Optimizer;

	class AnalysisDriver {
		protected:
			CompilationState& state;
			llvm::LLVMContext& context;

			// TODO: Not sure if pimpl is a good idea
			// Depends on whether the downstream code benefits from llvm or not
			Optimizer* opt = nullptr;

			parser::Stack ast;
			analysis::AnalysisState decls;
			SperoModule translation_unit = nullptr;

		protected:
			// Frontend: source -> AST
			void parseInput(parser::ParsingMode parser_mode, const std::string& input);

			// Analysis: AST -> MIR (LLVM IR)
			void analyzeAst();

			// Backend: MIR (LLVM IR) -> LLVM IR
			virtual void translateAstToLlvm();
			void optimizeLlvm();
			//void updateOptimizationLevel();

		public:
			AnalysisDriver(CompilationState& state, analysis::AllTypes& types);

			inline CompilationState& getState() {
				return state;
			}
	};

}