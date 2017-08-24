#include "interface/CompilationState.h"

namespace spero::compiler {
	CompilationState::CompilationState(char** fst, char** snd)
		: input_files{ fst, snd }, logger{ spdlog::stdout_color_mt("console") } {}


	// Input/Output files
	std::deque<std::string>& CompilationState::files() {
		return input_files;
	}


	// Time loggers
	void CompilationState::logTime() {
		timing.emplace_back(std::chrono::system_clock::now());
	}
	std::pair<TimePoint, TimePoint> CompilationState::getCycle(size_t i) {
		return std::make_pair(timing.at(i), timing.at(i + 1));
	}


	// Basic state querying
	bool CompilationState::deleteTemporaryFiles() {
		return true;
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

	size_t CompilationState::failed() {
		return nerrs;
	}
}
