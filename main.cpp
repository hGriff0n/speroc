#pragma warning(disable : 4503)

#include <iostream>
#include <unordered_map>

#include "compilation.h"
#include "parser/base.h"
#include "interface/cmd_line.h"
#include "util/strings.h"

template <class Stream>
Stream& getMultiline(Stream& in, spero::string::backing_type& s);
std::ostream& printAST(std::ostream& s, const spero::parser::Stack& stack);
void printAssembly(spero::compiler::gen::Assembler& asm_code);

// Helper function to run the interactive mode
void run_interpreter(spero::compiler::CompilationState& state, int& argc, char** argv) {
	using namespace spero;
	using namespace spero::parser;
	using compiler::ID;
	
	string::backing_type input;
	std::unordered_map<String, bool> flags{ { "compile", true }, { "interpret", true } };

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
					auto table = compiler::analyze(res, state);
					auto asm_code = compiler::backend(std::move(table), res, state);


					// Print out the generated assembly
					if (!state.failed()) {
						printAssembly(asm_code);

						if (flags["output"]) {
							compiler::codegen(asm_code, "interactive", "out.s", state, false);
						}

						if (flags["interpret"]) {
							compiler::interpret(asm_code);
							std::cout << std::endl;
						}

						std::remove("out.s");
					}
				}
			}

			if (!flags["ast"]) {
				printAST(std::cout << '\n', res);
			}

			state.reset();

		} catch (std::exception& e) {
			std::cout << e.what() << '\n';
		}
	}
}


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
Stream& getMultiline(Stream& in, spero::string::backing_type& out_str) {
	std::getline(in, out_str);
	if (out_str == ":q") {
		return in;
	}

	spero::string::backing_type tmp;
	while (std::getline(in, tmp)) {
		if (tmp == "") {
			return in;
		}

		out_str += "\n" + tmp;
	}

	return in;
}

// Helper function to print out the ast structure
std::ostream& printAST(std::ostream& s, const spero::parser::Stack& ast_stack) {
	for (const auto& node : ast_stack) {
		if (node) {
			node->prettyPrint(s, 0) << '\n';
		} else {
			s << "nullptr\n";
		}
	}

	return s << '\n';
}

void printAssembly(spero::compiler::gen::Assembler& asm_code) {
	asmjit::StringBuilder sb;
	asm_code.dump(sb);
	std::cout << sb.data() << '\n';
}


// Interpret the produced assembly code (this doesn't produce any code. "CodeHolder" is apparently completely empty
void spero::compiler::interpret(gen::Assembler& asm_code) {
	// Setup the runtime environment
	static asmjit::JitRuntime jt;

	// Prepare the assembly code for interpretation
	// I don't think this is quite accurate enough just yet (not sure what though)
	asm_code.makeIFunction();

	// Print the interpreted code
	printAssembly(asm_code);

	// Register the assembly code as an `int()` function
	gen::Assembler::Function fn;
	asmjit::Error err = jt.add(&fn, asm_code.get());

	if (err) {
		std::cout << err << ':' << asmjit::kErrorNoCodeGenerated << '\n';
	} else {
		int i = fn();
		std::cout << "result: " << i << '\n';
	}

	// TODO: Add printing of all other registers
	// TODO: Add re-entrant evaluation

	jt.release(fn);
}