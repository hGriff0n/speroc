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

	class CompilationState {
		std::deque<std::string> input_files;
		std::vector<time_point> timing;

		public:
			CompilationState(char** fst, char** snd) : input_files{ fst, snd } {}

			// Input/output files
			std::deque<std::string>& files() { return input_files; }
			//const std::string& output() { return output_file; }

			// Time loggers
			void logTime() { timing.emplace_back(std::chrono::system_clock::now()); }
			std::pair<time_point, time_point> getCycle(size_t i) { return std::make_pair(timing.at(i), timing.at(i + 1)); }
	};
}