#pragma once

#include <memory>
#include <deque>

#include <spdlog.h>
#include <logger.h>
#include <fmt/ostr.h>

#pragma warning(push, 0)
#pragma warning(disable:4996)
#include <llvm/IR/LLVMContext.h>
#pragma warning(pop)

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

	enum OptimizationLevel : char {
		NONE = '0',
		SMALL = 's',
		ALL
	};

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

		std::unique_ptr<llvm::LLVMContext> context;
		OptimizationLevel opt_level = OptimizationLevel::NONE;

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
			virtual std::string targetTriple() abstract;
			virtual std::string targetDataLayout() abstract;
			OptimizationLevel optimizationLevel();

			llvm::LLVMContext& getContext();
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
			: CompilationState{ fst, snd }, opts{ opts }
		{
			opt_level = static_cast<OptimizationLevel>(opts["O"].as<char>());
		}

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

		std::string targetTriple() {
			return "x86_64-pc-windows-msvc19.16.27025";
		}

		std::string targetDataLayout() {
			return "e-m:w-i64:64-f80:128-n8:16:32:64-S128";
		}
	};

	// Idea to rework the CompilationState interface
	namespace experimental {

		/*
		 * Class for managing `spdlog` logger instances and tracking usage statistics
		 *   We currently utilize the number of error/warning messages as a proxy for whether compilation succeeded
		 */
		class Logger {
			protected:
				size_t count[7] = { 0, 0, 0, 0, 0, 0, 0 };
				std::shared_ptr<spdlog::logger> logger;

				virtual spdlog::logger& getLogger(ID msg_id) = 0;

			public:
				template<class... Args>
				void log(ID msg_id, const char* fmt, Args&&... args) {
					count[msg_id]++;

					// Eventual Basic Code Flow for this function: logger(msg_id).log(level(msg_id), fmt, std::forward<Args>(args)...);
					getLogger(msg_id).log(msg_id, fmt, std::forward<Args>(args)...);
				}

				template<class... Args>
				void info(const char* fmt, Args&&... args) {
					log(ID::info, fmt, std::forward<Args>(args)...);
				}

				template<class... Args>
				void debug(const char* fmt, Args&&... args) {
					log(ID::debug, fmt, std::forward<Args>(args)...);
				}

				template<class... Args>
				void warn(const char* fmt, Args&&... args) {
					log(ID::warn, fmt, std::forward<Args>(args)...);
				}

				template<class... Args>
				void error(const char* fmt, Args&&... args) {
					log(ID::err, fmt, std::forward<Args>(args)...);
				}
		};

		class CompilationState : public Logger {
			private:
				std::unique_ptr<llvm::LLVMContext> context;

			protected:
				virtual spdlog::logger& getLogger(ID msg_id);

			public:

				// TODO: Interfaces to extract llvm-config information
				// TODO: Interfaces to extract spero-config information

				// TODO: Interfaces to collect statistics and timing information

				/*
				 * TODO: Add interface for reporting standardized error/warning messages
				 *   So instead of requiring `state.error("Attempt to use {} without declaration", ...);
				 *   we rather write `state.issueMessage(VARIABLE_NO_DECL, ...)`, which can handle "-Wall"
				 */

				// Error reporting
				inline size_t numWarnings() {
					return count[ID::warn];
				}
				size_t numErrors() {
					return count[ID::err];
				}
		};

	}
}

#undef abstract
