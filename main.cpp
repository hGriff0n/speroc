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

// Helper function to print out the ast structure
std::ostream& printAST(std::ostream& s, const spero::parser::Stack&);


/*
 * Run the compiler and it's command-line interface
 *
 * Currently doubles as an interactive parse-tree searcher if
 *   no args or a '-i' flag is passed to evaluation
 */
int main(int argc, char* argv[]) {
	using namespace spero;
	using namespace spero::parser;
	
	// Automatically add in "interactive" and "nodel" flags for debug running
	if (argc == 1) {
		argc = 3;
		argv = new char*[3]{ "speroc.exe", "-i", "-t" };
	}


	// Parse the command line arguments
	auto state = cmd::parse(argc, argv);


	// Interactive mode for testing ast creation/parsing runs
	if (state.opts["interactive"].as<bool>()) {
		std::string input;

		while (std::cout << "> " && getMultiline(std::cin, input)) {
			if (input == ":q") break;

			bool succ;
			parser::Stack res;

			try {
				if (input.substr(0, 2) == ":c") {
					if (state.files().size() == 0) state.files().push_back("");
					state.files()[0] = input.substr(3);

					if (succ = compile(state, res)) {
						std::cout << "Compilation Failed\n";
						continue;
					}
					
					// Print out the generated assembly
					std::ifstream out{ "out.s" };
					while (std::getline(out, input))
						std::cout << input << '\n';

				} else
					std::tie(succ, res) = compiler::parse(input, state);

				std::cout << "Parsing Succeeded: " << (succ ? "false" : "true") << '\n';
				printAST(std::cout, res);

			} catch (std::exception& e) {
				std::cout << e.what() << '\n';
			}
		}

	// Compiler run
	} else {
		parser::Stack res;

		bool successful = compile(state, res);

		// Perform error reporting if not successful
		// TODO:

		return successful;
	}
}


/*
 * Implementation of compilation function
 * Defined in 'main.cpp' for reasons
 */
bool spero::compile(spero::compiler::CompilationState& state, spero::parser::Stack& stack) {
	using namespace spero;

	// TODO: Initialize timing and other compilation logging structures
	// TODO: Move to a more explicit check (ie. how many errors does the structure have?)
	bool failed;


	/*
	 * Perform parsing and initial recognization checks
	 */
	state.logTime();
	std::tie(failed, stack) = compiler::parseFile(state.files()[0], state);
	state.logTime();


	/*
	 * Run through the analysis stages
	 *
	 * Don't mark this as a separate "phase" for timing purposes
	 * The sub-phases perform their own timing passes
	 */
	auto ir = compiler::analyze(std::move(stack), state, failed);

	
	/*
	 * Emit final codegen processes
	 *   Note: If control reaches here, compilation should not fail
	 */
	if (!failed) {
		state.logTime();
		compiler::codegen(ir, state.files()[0], "out.s", state);
		state.logTime();


		// Forward creation of the actual executable to some other compiler
		state.logTime();
		failed = system((ASM_COMPILER" out.s -o " + state.output()).c_str());
		state.logTime();


		// Delete the temporary file
		bool del_files = state.deleteTemporaryFiles();
		if (state.deleteTemporaryFiles())
			std::remove("out.s");
	}

	return failed;
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


std::ostream& printAST(std::ostream& s, const spero::parser::Stack& stack) {
	for (const auto& node : stack)
		if (node) node->prettyPrint(s, 0) << '\n';
		else s << "nullptr\n";

		return s << '\n';
}