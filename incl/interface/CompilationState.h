#pragma once

#include <memory>
#include <deque>

#include <spdlog.h>
#include <logger.h>
#include <fmt/ostr.h>

#include "parser/base.h"
#include "util/time.h"

#define abstract =0;
#define GET_PERMISSIONS(x) auto& [parser_mode, do_compile, link, interpret] = (x).getPermissions()


// Forward Declarations
namespace spero::parser {
	using Stack = std::deque<compiler::ptr<compiler::ast::Ast>>;

	enum class ParsingMode {
		NONE,
		FILE,
		STRING
	};
}


namespace spero::compiler {
	using ID = spdlog::level::level_enum;
	using CompilationPermissions = std::tuple<parser::ParsingMode, bool, bool, bool>;

	/*
	 * Base class to define the interaction point for querying the
	 * specified compilation state from all parts of the compiler
	 */
	class CompilationState {
		std::deque<std::string> input_files;
		std::deque<std::pair<std::string, util::TimeData>> timing;
		int nerrs = 0;

		std::shared_ptr<spdlog::logger> logger;
		CompilationPermissions permissions;

		spdlog::logger& getLogger(ID msg_id);

		public:
			CompilationState(char** fst, char** snd);

			// Input/Output files
			std::deque<std::string>& files();
			virtual const std::string& output() abstract;

			// Timing interface
			// NOTE: Improve this if you want to, my purposes are really simplistic
			util::Timer timer(std::string phase);
			const std::deque<std::pair<std::string, util::TimeData>>& getTiming() const;

			// Error reporting/collection
			// TODO: Add in more complex logger manipulations (particularly change formatting)
			// TODO: Add in rough ability to "craft" messages from component parts (instead of expecting me to do it)
			template<class... Args>
			void log(ID msg_id, const char* fmt, Args&&... args) {
				// Eventual Basic Code Flow for this function: logger(msg_id).log(level(msg_id), fmt, std::forward<Args>(args)...);
				nerrs += (msg_id == ID::err);
				getLogger(msg_id).log(msg_id, fmt, std::forward<Args>(args)...);
			}


			// State Querying
			virtual bool deleteTemporaryFiles() abstract;
			virtual bool showLogs() abstract;
			virtual bool produceExe() abstract;
			int failed() const;
			void reset();


			// Permission System
			// This system is used to inform the compilation process about which steps should be taken
			// With this, we are able to use the same compilation path for normal usage and for the repl
			CompilationPermissions& getPermissions();
			template<class... Args>
			void setPermissions(Args&&... args) {
				permissions = CompilationPermissions{ args... };
			}
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

#undef abstract
