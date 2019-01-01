#pragma once

#include "driver/AnalysisDriver.h"

namespace spero::compiler {

	class CompilationDriver : public AnalysisDriver {
		protected:
			// Backend: LLVM IR -> Executable
			void writeLlvmToFile();
			void triggerClangCompile();

		public:
			CompilationDriver(CompilationState& state, analysis::AllTypes& types);

			bool compile();
	};

}