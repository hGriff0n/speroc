#pragma warning(disable : 4503)

#include <iostream>
#include <unordered_map>

#include "compilation.h"
#include "parser/base.h"
#include "interface/cmd_line.h"
#include "util/strings.h"

template<class Stream>
Stream& getMultiline(Stream& in, spero::string::backing_type& s);
std::ostream& printAST(std::ostream& s, const spero::parser::Stack& stack);
void printAssembly(std::ostream&, spero::compiler::gen::Assembler& asm_code);


// Helper function to run the interactive mode
void run_interpreter(cxxopts::Options& opts, spero::compiler::CompilationState& state, int& argc, char** argv) {
	using namespace spero;
	using namespace spero::parser;
	using compiler::ID;
	
	string::backing_type input;
	std::unordered_map<String, bool> flags{ { "compile", true }, { "interpret", false }, { "ast", true }, { "asm", true } };

	GET_PERMISSIONS(state);
	state.files().push_back("");

	while (std::cout << "> " && getMultiline(std::cin, input)) {
		parser::Stack res;

		// compile file
		if (auto command = input.substr(0, 2); command == ":c") {
			state.files()[0] = input.substr(3);

			parser_mode = ParsingMode::FILE;	link = true;
			do_compile = true;				interpret = false;

		// load file
		} else if (command == ":l") {
			state.files()[0] = input.substr(3);

			parser_mode = ParsingMode::FILE;
			do_compile = false;

		// set flag
		} else if (command == ":s") {
			for (auto flag : util::split(input.substr(3), ',')) {
				flags[flag] = !flags[flag];
			}

			continue;

		// quit
		} else if (command == ":q") {
			break;

		// handle string input
		} else {
			state.files()[0] = input;

			parser_mode = ParsingMode::STRING;	link = false;
			do_compile = flags["compile"];	interpret = flags["interpret"];
		}


		// Run the input through the internal compilation loop (printing the ast and assembler)
		compile(state, res, [&](auto&& ir) {
			if constexpr (std::is_same_v<std::decay_t<decltype(ir)>, parser::Stack>) {
				if (flags["ast"]) {
					printAST(std::cout << '\n', ir);
				}
			}

			if constexpr (std::is_same_v<std::decay_t<decltype(ir)>, compiler::gen::Assembler>) {
				if (flags["asm"]) {
					printAssembly(std::cout, ir);
				}
			}
		});

		std::cout << std::endl;
		state.reset();
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
	auto opts = cmd::getOptions();
	auto state = cmd::parse(opts, argc, argv);

	// Compiler run
	if (!state.opts["interactive"].as<bool>()) {
		state.setPermissions(parser::ParsingMode::FILE, true, true, false);

		parser::Stack res;
		compile(state, res);

		return state.failed();

	// Interactive mode
	} else {
		state.setPermissions(parser::ParsingMode::STRING, true, false, true);
		return run_interpreter(opts, state, argc, argv), 0;
	}
}



/*
 * Implementation of basic repl loop, or a Spero interpreter
 *
 * TODO: It may be beneficial to move this to a separate file
 */
// Wrapper around std::getline that waits for [ENTER] to be hit twice before accepting input
template<class Stream>
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

void printAssembly(std::ostream& s, spero::compiler::gen::Assembler& asm_code) {
	asmjit::StringBuilder sb;
	asm_code.dump(sb);
	s << sb.data() << '\n';
}


// Interpret the produced assembly code (this doesn't produce any code. "CodeHolder" is apparently completely empty
void spero::compiler::interpret(gen::Assembler& asm_code) {
	// Setup the runtime environment
	static asmjit::JitRuntime jt;

	// Prepare the assembly code for interpretation
	// I don't think this is quite accurate enough just yet (not sure what though)
	asm_code.makeIFunction();

	// Print the interpreted code
	//printAssembly(std::cout, asm_code);

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