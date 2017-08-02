#pragma once

#include <chrono>
#include <memory>
#include <deque>
#include <spdlog.h>
#include "diagnostic.h"

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
	using time_point = std::chrono::system_clock::time_point;

	/*
	 * Base class to define the interaction point for querying the
	 * specified compilation state from all parts of the compiler
	 */
	class CompilationState {
		std::deque<std::string> input_files;
		std::deque<time_point> timing;
		std::deque<Diagnostic> diags;
		size_t nerrs;

		public:
			CompilationState(char**, char**);

			// Input/Output files
			std::deque<std::string>& files();
			virtual const std::string& output() abstract;

			// Time loggers
			void logTime();
			std::pair<time_point, time_point> getCycle(size_t);

			// Basic state querying
			virtual bool deleteTemporaryFiles() abstract;
			virtual bool showLogs() abstract;

			// Error reporting/collection
			DiagnosticBuilder log(std::string);
			DiagnosticBuilder error(std::string);
			DiagnosticBuilder warn(std::string);
			void clearDiagnostics();
			size_t failed();

			// TODO: Temp Function. Please Delete
			template<class Stream>
			void printErrors(Stream& s) {
				bool show_logs = showLogs();

				for (auto& diag : diags) {
					if (show_logs || diag.level != Diagnostic::Level::LOG) {
						s << '[' << '_' << "] = " << diag.message << diag.location() << '\n';
					}
				}
			}

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
			return !opts["asm"].as<bool>();
		}
	};
}