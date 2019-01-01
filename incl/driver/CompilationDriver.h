#pragma once

#include "driver/AnalysisDriver.h"

namespace spero::compiler {

	class CompilationDriver : public AnalysisDriver {
		protected:
			// Backend: LLVM IR -> Executable
			void writeLlvmToFile(bool perform_compilation);
			void triggerClangCompile(bool perform_compilation);

		public:
			CompilationDriver(CompilationState& state, analysis::AllTypes& types);

			bool compile();
	};

}