#pragma once

#include <chrono>
#include <memory>
#include <deque>


// Forward Declarations
namespace spero::compiler {
	template<class T> using ptr = std::unique_ptr<T>;
}

namespace spero::compiler::ast {
	struct Ast;
}

namespace spero::parser {
	using Stack = std::deque<compiler::ptr<compiler::ast::Ast>>;

	// Helper function to print out the ast
	std::ostream& printAST(std::ostream&, const Stack&);
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

		public:
			CompilationState(char**, char**);

			// Input/Output files
			std::deque<std::string>& files();
			virtual const std::string& output() =0;

			// Time loggers
			void logTime();
			std::pair<time_point, time_point> getCycle(size_t);

			// Basic state querying
			virtual bool deleteTemporaryFiles();
	};

	// Special subtype to allow for passing around the parsed
	// "cxxopts::Options" class without introducing a dependency
	// on cxxopts in subsequent header files that use CompilationState
	template<class Option>
	struct OptionState : CompilationState {
		const Option opts;

		OptionState(char** fst, char** snd, Option&& opts)
			: CompilationState{ fst, snd }, opts{ opts } {}

		// Overrides
		const std::string& output() {
			return opts["out"].as<std::string>();
		}

		bool deleteTemporaryFiles() {
			return !opts["nodel"].as<bool>();
		}
	};
}