#pragma warning(disable : 4503)

#include <iostream>
#include <unordered_map>
#include <cctype>

#include "compiler.h"
#include "parser/ast.h"

#include "cmd_line.h"
#include "util/utils.h"


// TODO: Convert to using llvm (if only it worked on windows)
#define ASM_COMPILER "g++"


// Wrapper around std::getline that waits for [ENTER] to be hit twice before accepting input
template <class Stream>
Stream& getMultiline(Stream&, std::string&);
std::deque<std::string> split(std::string, char=',');

// Helper function to print out the ast structure
std::ostream& printAST(std::ostream& s, const spero::parser::Stack&);

// Exported interpreter functions (implemented in Rust)
extern "C" {
	void interpret_file(const char*);
	void interpret_string(const char*);
}


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
		argv = new char*[3]{ "speroc.exe", "-i", "--nodel" };
	}


	// Parse the command line arguments
	auto state = cmd::parse(argc, argv);


	// Interactive mode for testing ast creation/parsing runs
	if (state.opts["interactive"].as<bool>()) {
		std::string input;
		std::unordered_map<std::string, bool> flags;

		while (std::cout << "> " && getMultiline(std::cin, input)) {
			try {
				parser::Stack res;

				std::string command = input.substr(0, 2);
				state.clearDiagnostics();

				// Compile a file
				if (command == ":c") {
					if (state.files().size() == 0) { state.files().push_back(""); }
					state.files()[0] = input.substr(3);

					compile(state, res);

					// Print out the generated assembly
					if (!state.failed()) {
						std::ifstream out{ "out.s" };
						while (std::getline(out, input)) {
							std::cout << input << '\n';
						}
					}

				// Load and parse a file
				} else if (command == ":l") {
					if (state.files().size() == 0) { state.files().push_back(""); }
					state.files()[0] = input.substr(3);

					bool failed;
					std::tie(failed, res) = compiler::parseFile(state.files()[0], state);
					if (failed) {
						state.error("Parsing of the input failed");
					}

				// Modify the command line arguments
				} else if (command == ":a") {
					auto args = split(input.substr(3), ' ');
					argv = new char*[args.size() + 2];
					argc = 0;
					
					argv[argc++] = "speroc.exe";
					argv[argc++] = "-i";
					for (auto& arg : args) {
						argv[argc++] = const_cast<char*>(arg.c_str());
					}

					state = cmd::parse(argc, argv);

				// Set an interactive flag
				} else if (command == ":s") {
					for (auto flag : split(input.substr(3))) {
						// TODO: Trim whitespace from 'flag'
						flags[flag] = !flags[flag];
					}

					continue;

				// Quit
				} else if (command == ":q") {
					break;

				// Run the interactive mode
				} else {
					bool failed;
					std::tie(failed, res) = compiler::parse(input, state);
					
					// Stop early if parsing failed
					if (failed) {
						state.error("Parsing of the input failed");

					} else if (flags["compile"]) {
						// Analyze and compile the code
						auto ir = compiler::analyze(std::move(res), state);
						compiler::codegen(ir, "interactive", "out.s", state, false);

						// Print out the generated assembly
						if (!state.failed()) {
							std::ifstream out{ "out.s" };
							while (std::getline(out, input)) {
								std::cout << input << '\n';
							}

							// TODO: Allow interpretation without requiring compilation displaying
							if (flags["interpret"]) {
								interpret_file("out.s");
								std::cout << std::endl;
							}

							std::remove("out.s");
						}

						// Reset `res` for future usage
						res = std::move(ir);
					}
				}

				state.printErrors(std::cout);
				printAST(std::cout << '\n', res);

			} catch (std::exception& e) {
				std::cout << e.what() << '\n';
			}
		}

	// Compiler run
	} else {
		parser::Stack res;
		try {
			compile(state, res);

		} catch (std::exception& e) {
			state.log(e.what());
		}

		if (state.failed()) {
			state.printErrors(std::cout);
		}

		return state.failed();
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

		if (failed) {
			state.error("Parsing of the input failed");
		}
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
		if (system((ASM_COMPILER" out.s -o " + state.output()).c_str())) {
			state.error("Compilation of `out.s` failed");
		}
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


#include <sstream>
#include <algorithm>
#include <iterator>
std::deque<std::string> split(std::string str, char ch) {
	std::istringstream iss(str);
	std::deque<std::string> ret;
	std::string in;

	while (std::getline(iss, in, ch)) {
		// Determine the chunk of the string that isn't space
		auto front = std::find_if(in.begin(), in.end(), [](int ch) { return !std::isspace(ch); });
		auto back = std::find_if(front, in.end(), [](int ch) { return std::isspace(ch); });

		ret.emplace_back(front, back);
	}
	
	return std::move(ret);
}
