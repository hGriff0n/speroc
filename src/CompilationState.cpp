#include "util/CompilationState.h"

namespace spero::compiler {
	CompilationState::CompilationState(char** fst, char** snd) : input_files{ fst, snd } {}


	// Input/Output files
	std::deque<std::string>& CompilationState::files() {
		return input_files;
	}
	//const std::string& output() { return output_file; }


	// Time loggers
	void CompilationState::logTime() {
		timing.emplace_back(std::chrono::system_clock::now());
	}
	std::pair<time_point, time_point> CompilationState::getCycle(size_t i) {
		return std::make_pair(timing.at(i), timing.at(i + 1));
	}

	
	// Basic state querying
	bool CompilationState::deleteTemporaryFiles() {
		return true;
	}


	// Error reporting/collection
	void CompilationState::reportError(std::string msg) {
		diags.emplace_back(Diagnostic::Level::ERROR, msg);
	}
	void CompilationState::addWarning(std::string msg) {
		diags.emplace_back(Diagnostic::Level::WARNING, msg);
	}
	void CompilationState::clearDiagnostics() {
		diags.clear();
	}
	size_t CompilationState::failed() {
		return diags.size();
	}
}
