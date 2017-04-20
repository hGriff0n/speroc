#pragma warning(disable : 4503)

#include <iostream>
#include "compiler.h"

#include "parser/ast.h"
#include "cmd_line.h"
#include "util/utils.h"

#define ASM_COMPILER "gcc"

// Wrapper around std::getline that waits for [ENTER] to be hit twice before accepting input
template <class Stream>
Stream& getMultiline(Stream&, std::string&);


/*
 * Run the compiler and it's command-line interface
 *
 * Currently doubles as an interactive parse-tree searcher if
 *   no args or a '-i' flag is passed to evaluation
 */
int main(int argc, char* argv[]) {
	using namespace spero;
	using namespace spero::parser;

	// Parse the command line arguments
	auto opts = cmd::getOptions();
	auto state = cmd::parse(opts, argc, argv);


	// Interactive mode for testing ast creation/parsing runs
	if (state.files().size() == 0 || opts["inter"].as<bool>()) {
		std::string input;

		while (std::cout << "> " && getMultiline(std::cin, input)) {
			if (input == ":q") break;

			bool succ;
			parser::Stack res;

			if (input.substr(0, 2) == ":c") {
				state.files()[0] = input.substr(3);

				if (!compile(state, res, "out.exe")) {
					std::cout << "Compilation Failed\n";
					continue;
				}

			} else
				std::tie(succ, res) = compiler::parse(input, state);

			std::cout << "Parsing Succeeded: " << (succ ? "true" : "false") << '\n';
			printAST(std::cout, res);
		}

	// Compiler run
	} else {
		parser::Stack res;

		bool success = compile(state, res, opts["out"].as<std::string>());

		return !success;
	}
}

bool spero::compile(spero::compiler::CompilationState& state, spero::parser::Stack& stack, std::string out_file) {
	using namespace spero;

	// TODO: Initialize timing and other compilation logging structures
	bool succ;


	/*
	 * Perform parsing and initial recognization checks
	 */
	state.logTime();
	std::tie(succ, stack) = compiler::parseFile(state.files()[0], state);
	state.logTime();


	/*
	 * Run through the analysis stages
	 */
	auto ir = compiler::analyze(stack, state);

	
	/*
	 * Emit final codegen processes
	 *   Note: If control reaches here, compilation should not fail
	 */
	if (succ) {
		state.logTime();
		compiler::codegen(ir, state.files()[0], "out.s", state);
		state.logTime();


		// Forward creation of the actual executable to some other compiler
		state.logTime();
		system((ASM_COMPILER" out.s -o " + out_file).c_str());
		state.logTime();
	}

	return succ;
}

template <class Stream>
Stream& getMultiline(Stream& in, std::string& s) {
	std::getline(in, s);
	if (s == ":q") return in;

	std::string tmp;
	while (std::getline(in, tmp)) {
		if (tmp == "") return in;

		s += "\n" + tmp;
	}

	return in;
}