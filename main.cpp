#pragma warning(disable : 4503)

#include <iostream>
#include <unordered_map>

#include "compiler.h"
#include "parser/ast.h"

#include "cmd_line.h"
#include "util/utils.h"


// TODO: Convert to using llvm (if only it worked on windows)
#define ASM_COMPILER "g++"


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
		std::unordered_map<std::string, bool> flags;

		while (std::cout << "> " && getMultiline(std::cin, input)) {
			try {
				bool succ;
				parser::Stack res;

				std::string command = input.substr(0, 2);

				// Compile a file
				if (command == ":c") {
					if (state.files().size() == 0) { state.files().push_back(""); }
					state.files()[0] = input.substr(3);

					if (succ = compile(state, res)) {
						std::cout << "Compilation Failed " << state.failed() << '\n';
						continue;
					}

					// Print out the generated assembly
					std::ifstream out{ "out.s" };
					while (std::getline(out, input)) {
						std::cout << input << '\n';
					}

				// Load and parse a file
				} else if (command == ":l") {
					if (state.files().size() == 0) { state.files().push_back(""); }
					state.files()[0] = input.substr(3);

					std::tie(succ, res) = compiler::parseFile(state.files()[0], state);

				// Set an interactive flag
				} else if (command == ":s") {
					auto flag = input.substr(3);
					flags[flag] = !flags[flag];
					continue;

				// Quit
				} else if (command == ":q") {
					break;

				// Run the interactive mode
				} else {
					std::tie(succ, res) = compiler::parse(input, state);

					// If I want to see the generated assembly
					if (flags["compile"]) {
						// Analyze and compile the code
						auto ir = compiler::analyze(std::move(res), state);
						compiler::codegen(ir, "interactive", "out.s", state, false);

						// Print out the generated assembly
						std::ifstream out{ "out.s" };
						while (std::getline(out, input)) {
							std::cout << input << '\n';
						}
						std::remove("out.s");

						// Reset `res` and `succ` for future usage
						res = std::move(ir);
						succ = state.failed();
					}
				}

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

		// TODO: Perform error reporting if not successful

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


	/*
	 * Perform parsing and initial recognization checks
	 */
	{
		size_t failed;

		state.logTime();
		std::tie(failed, stack) = compiler::parseFile(state.files()[0], state);
		state.logTime();

		state.setStatus(failed);
	}


	/*
	 * Run through the analysis stages
	 *
	 * Don't mark this as a separate "phase" for timing purposes
	 * The sub-phases perform their own timing passes
	 */
	auto ir = (!state.failed())
		? compiler::analyze(std::move(stack), state)
		: spero::compiler::IR_t{};


	/*
	 * Generate the boundary ir for the external tools
	 * 
	 * speroc does not handle the generation of executables
	 * and other binary files, prefering to pass those stages
	 * off to some existing system tool that is guaranteed to work
	 */
	if (!state.failed()) {
		state.logTime();
		compiler::codegen(ir, state.files()[0], "out.s", state);
		state.logTime();
	}


	/*
	 * Send the boundary ir off to the final compilation phase
	 *
	 * NOTE: Currently speroc uses `g++` for final compilation
	 *   I will be moving over to LLVM (and the LLVM IR) in the future
	 */
	if (!state.failed() && state.produceExe()) {
		state.logTime();
		state.setStatus(system((ASM_COMPILER" out.s -o " + state.output()).c_str()));
		if (state.failed()) { state.setStatus(4); }
		state.logTime();


		// Delete the temporary file
		if (state.deleteTemporaryFiles()) {
			std::remove("out.s");
		}
	}

	return state.failed();
}


template <class Stream>
Stream& getMultiline(Stream& in, std::string& s) {
	std::getline(in, s);
	if (s == ":q") {
		return in;
	}

	std::string tmp;
	while (std::getline(in, tmp)) {
		if (tmp == "") {
			return in;
		}

		s += "\n" + tmp;
	}

	return in;
}


std::ostream& printAST(std::ostream& s, const spero::parser::Stack& stack) {
	for (const auto& node : stack) {
		if (node) {
			node->prettyPrint(s, 0) << '\n';
		} else {
			s << "nullptr\n";
		}
	}

	return s << '\n';
}
