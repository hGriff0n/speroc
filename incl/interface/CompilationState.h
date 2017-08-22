#pragma once

#include <chrono>
#include <memory>
#include <deque>
#include <spdlog.h>
#include <logger.h>
#include "pegtl/position.hpp"

#define abstract =0;


// Forward Declarations
namespace spero::compiler {
	template<class T> using ptr = std::unique_ptr<T>;
}

namespace spero::compiler::ast {
	struct Ast;
}

namespace spero::parser {
	using Stack = std::deque<compiler::ptr<compiler::ast::Ast>>;
}

namespace spero::compiler {
	using TimePoint = std::chrono::system_clock::time_point;
	using ID = spdlog::level::level_enum;

	/*
	 * Base class to define the interaction point for querying the
	 * specified compilation state from all parts of the compiler
	 */
	class CompilationState {
		std::deque<std::string> input_files;
		std::deque<TimePoint> timing;
		size_t nerrs = 0;

		std::shared_ptr<spdlog::logger> logger;

		spdlog::logger& getLogger(ID msg_id);

		public:
			CompilationState(char**, char**);

			// Input/Output files
			std::deque<std::string>& files();
			virtual const std::string& output() abstract;

			// Time loggers (Unused)
			// TODO: Improve internal 'benchmarking' interface
			void logTime();
			std::pair<TimePoint, TimePoint> getCycle(size_t);

			// Basic state querying
			virtual bool deleteTemporaryFiles() abstract;
			virtual bool showLogs() abstract;

			// Error reporting/collection
			// TODO: Add in more complex logger manipulations (particularly change formatting)
			// TODO: Add in rough ability to "craft" messages from component parts (instead of expecting me to do it)
			template<class... Args>
			void log(ID msg_id, const char* fmt, Args&&... args) {
				// Eventual Basic Code Flow for this function: logger(msg_id).log(level(msg_id), fmt, std::forward<Args>(args)...);
				if (msg_id == ID::err) { ++nerrs; }

				getLogger(msg_id).log(msg_id, fmt, std::forward<Args>(args)...);
			}

			size_t failed();

			// Temp function for formatting position
			static std::string location(tao::pegtl::position);

			// Compilation Stage Control
			virtual bool produceExe() abstract;
	};

	// Special subtype to allow for passing around the parsed
	// "cxxopts::Options" class without introducing a dependency
	// on cxxopts in subsequent header files that use CompilationState
	template<class Option>
	struct OptionState : CompilationState {
		Option opts;

		OptionState(char** fst, char** snd, Option&& opts)
			: CompilationState{ fst, snd }, opts{ opts } {}

		// Overrides
		const std::string& output() {
			return opts["out"].as<std::string>();
		}

		bool deleteTemporaryFiles() {
			return !opts["nodel"].as<bool>();
		}

		bool showLogs() {
			return opts["showlog"].as<bool>();
		}

		bool produceExe() {
			return !opts["stop"].as<bool>();
		}
	};
}