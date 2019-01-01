#pragma once

#include "driver/AnalysisDriver.h"

#pragma warning(push, 0)
#pragma warning(disable:4996)
#include <llvm/IR/Module.h>
#include <ExecutionEngine/Interpreter/Interpreter.h>
#pragma warning(pop)

namespace spero::compiler {

	class ReplDriver : public AnalysisDriver {
	protected:
		llvm::Interpreter interpreter;

	protected:
		// Frontend: source -> AST
		void addInterpreterAstTransformations();

		// Backend: Run LLVM IR
		void interpretLlvm();

		// Prepare the repl for the next iteration
		bool reset();

	public:
		ReplDriver(CompilationState& state, analysis::AllTypes& types);

		template<class IrHookFn>
		bool interpret(IrHookFn&& ir_hook) try {
			const GET_PERMISSIONS(state);

			// frontend
			parseInput(parser_mode, state.files()[0]);
			if (!do_compile) {
				ir_hook(ast);
				return true;
			}

			// analysis
			addInterpreterAstTransformations();
			analyzeAst();
			if (!state.failed()) {
				ir_hook(ast);
			}

			// backend
			translateAstToLlvm();
			if (!state.failed()) {
				ir_hook(ast);
			}

			// optimizations
			bool do_optimizations = false;
			if (do_optimizations) {
				optimizeLlvm();
			}

			// interpretation
			interpretLlvm();

			return reset();

		} catch (std::exception& e) {
			state.log(ID::err, e.what());
			return true;
		}
	};

//#undef GET_PERMISSIONS
}
