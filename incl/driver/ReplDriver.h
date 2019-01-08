#pragma once

#include "driver/AnalysisDriver.h"

namespace spero::compiler {
	
	class ReplDriver : public AnalysisDriver {
		protected:
			// Frontend: source -> AST
			void addInterpreterAstTransformations();

			// Backend: Run LLVM IR
			void interpretLlvm();

			// Prepare the repl for the next iteration
			bool reset();

		public:
			ReplDriver(CompilationState& state);

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
				ir_hook(ast);

				// backend
				translateAstToLlvm();
				if (!state.failed()) {
					ir_hook(translation_unit);
				}

				// interpretation
				optimizeLlvm();
				interpretLlvm();

				// Prepare the interpreter for the next run
				return reset();

			} catch (std::exception& e) {
				state.log(ID::err, e.what());
				return true;
			}
	};

//#undef GET_PERMISSIONS
}
