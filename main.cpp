#pragma warning(disable : 4503)

#include <iostream>
#include <unordered_map>
//#include <asmtk/asmtk.h>

#include "compilation.h"
#include "parser/base.h"
#include "interface/cmd_line.h"
#include "util/strings.h"

// TODO: Convert to llvm
#define CLANG_COMMAND "clang -masm=intel"
#define GCC_COMMAND "g++ -masm=intel"
#define ASM_COMPILER GCC_COMMAND


/*
 * Implementation of compilation function
 * Defined in 'main.cpp' for reasons
 *
 * TODO: Move this definition to 'compiler.cpp'
 */
bool spero::compile(spero::compiler::CompilationState& state, spero::parser::Stack& stack) {
	using namespace spero;

	// TODO: Initialize timing and other compilation logging structures


	/*
	 * Perform parsing and initial AST assembly
	 */
	state.logTime();
	stack = compiler::parseFile(state.files()[0], state);
	state.logTime();


	/*
	 * Run through the analysis stages
	 *
	 * Don't mark this as a separate "phase" for timing purposes
	 * The sub-phases perform their own timing passes
	 */
	auto ir = (!state.failed())
		? compiler::analyze(std::move(stack), state)
		: spero::compiler::MIR_t{};


	/*
	 * Run through the backend optimizations
	 *
	 * Don't mark this as a separate "phase" for timing purposes
	 * The sub-phases perform their own timing passes
	 */
	auto asmCode = (!state.failed())
		? compiler::backend(ir, state)
		: spero::compiler::gen::Assembler{};

	/*
	 * Generate the boundary ir for the external tools
	 *
	 * speroc does not handle the generation of executables
	 * and other binary files, prefering to pass those stages
	 * off to some system tool that is guaranteed to work
	 */
	if (!state.failed()) {
		state.logTime();
		compiler::codegen(asmCode, state.files()[0], "out.s", state);
		state.logTime();
	}


	/*
	 * Send the boundary ir off to the final compilation phase
	 *
	 * NOTE: Currently speroc uses `g++` for final compilation
	 *   I will be moving over to LLVM (and LLVM IR) in the future
	 */
	if (!state.failed() && state.produceExe()) {
		state.logTime();
		if (system((ASM_COMPILER" out.s -o " + state.output()).c_str())) {
			state.log(compiler::ID::err, "Compilation of `{}` failed", state.output());
		}
		state.logTime();


		// Delete the temporary file
		if (state.deleteTemporaryFiles()) {
			std::remove("out.s");
		}
	}

	return state.failed();
}


// Helper function to run the interactive mode
void run_interpreter(spero::compiler::CompilationState&, int&, char**);


/*
 * Run the compiler and it's command-line interface
 *
 * Currently doubles as an interactive parse-tree searcher if
 *   no args or a '-i' flag is passed to evaluation
 */
int main(int argc, char* argv[]) {
	using namespace spero;

	// Parse the command line arguments
	auto state = cmd::parse(argc, argv);


	// Compiler run
	if (!state.opts["interactive"].as<bool>()) {
		parser::Stack res;
		try {
			compile(state, res);

		} catch (std::exception& e) {
			state.log(compiler::ID::info, e.what());
		}

		return state.failed();

	// Interactive mode
	} else {
		return run_interpreter(state, argc, argv), 0;
	}
}



/*
 * Implementation of basic repl loop, or a Spero interpreter
 *
 * TODO: It may be beneficial to move this to a separate file
 */
// Wrapper around std::getline that waits for [ENTER] to be hit twice before accepting input
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

// Helper function to print out the ast structure
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


// Exported interpreter functions (implemented in Rust)
extern "C" {
	void interpret_file(const char*);
	void interpret_string(const char*);
}


void run_interpreter(spero::compiler::CompilationState& state, int& argc, char** argv) {
	using namespace spero;
	using namespace spero::parser;
	using compiler::ID;

	std::string input;
	std::unordered_map<std::string, bool> flags;

	while (std::cout << "> " && getMultiline(std::cin, input)) {
		try {
			parser::Stack res;

			// Compile a file
			if (auto command = input.substr(0, 2); command == ":c") {
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
				res = compiler::parseFile(state.files()[0], state);

			// Modify the command line arguments
			} else if (command == ":a") {
				auto args = util::split(input.substr(3), ' ');
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
				for (auto flag : util::split(input.substr(3), ',')) {
					flags[flag] = !flags[flag];
				}

				continue;

			// Quit
			} else if (command == ":q") {
				break;

			// Run the interactive mode
			} else {
				res = compiler::parse(input, state);

				// Stop early if parsing failed
				if (!state.failed() && flags["compile"]) {
					// Analyze and compile the code
					auto ir = compiler::analyze(std::move(res), state);
					auto asmCode = compiler::backend(ir, state);

					// TODO: Insert the interpret check before the codegen output
					
					// TODO: I don't need to output to a file anymore

					compiler::codegen(asmCode, "interactive", "out.s", state, false);

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

			printAST(std::cout << '\n', res);

		} catch (std::exception& e) {
			std::cout << e.what() << '\n';
		}
	}
}