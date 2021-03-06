#include "interface/CompilationState.h"

namespace spero::compiler {
	CompilationState::CompilationState(char** fst, char** snd)
		: input_files{ fst, snd }, logger{ spdlog::stdout_color_mt("console") },
		  context{ std::make_unique<llvm::LLVMContext>() } {}


	// Input/Output files
	std::deque<std::string>& CompilationState::files() {
		return input_files;
	}


	// Time loggers
	util::Timer CompilationState::timer(std::string phase) {
		return util::Timer{ timing.emplace_back(phase, util::TimeData{}).second };
	}
	const std::deque<std::pair<std::string, util::TimeData>>& CompilationState::getTiming() const {
		return timing;
	}



	// Basic state querying
	bool CompilationState::deleteTemporaryFiles() {
		return true;
	}
	llvm::LLVMContext& CompilationState::getContext() {
		return *context;
	}
	OptimizationLevel CompilationState::optimizationLevel() {
		return opt_level;
	}
	void CompilationState::flipOptimization() {
		opt_level = (opt_level == OptimizationLevel::NONE) ? OptimizationLevel::ALL : OptimizationLevel::NONE;
	}

	/* Example code on how to create a "multi-sink" logger
	std::vector<spdlog::sink_ptr> sinks;
	auto stdout_sink = spdlog::sinks::stdout_sink_mt::instance();
	auto color_sink = std::make_shared<spdlog::sinks::ansicolor_sink>(stdout_sink);
	sinks.push_back(color_sink);
	// Add more sinks here, if needed.
	auto combined_logger = std::make_shared<spdlog::logger>("main", begin(sinks), end(sinks));
	spdlog::register_logger(combined_logger);
	 */


	// Error reporting/collection
	spdlog::logger& CompilationState::getLogger(ID msg_id) {
		return *logger;
	}

	int CompilationState::failed() const {
		return nerrs;
	}

	void CompilationState::reset() {
		nerrs = 0;
	}

	CompilationPermissions& CompilationState::getPermissions() {
		return permissions;
	}
}
