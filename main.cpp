#pragma warning(disable : 4503)

#include <iostream>
#include <unordered_map>

#pragma warning(push, 0)
#pragma warning(disable:4996)
#include <llvm/IR/Module.h>
#pragma warning(pop)

#include "interface/cmd_line.h"
#include "util/strings.h"

#include "driver/CompilationDriver.h"
#include "driver/ReplDriver.h"

template<class Stream>
Stream& getMultiline(Stream& in, spero::string::backing_type& s);
std::ostream& printAST(std::ostream& s, const spero::parser::Stack& stack);
void printAssembly(std::ostream&, std::unique_ptr<llvm::Module>& llvm_code);


// Helper function to run the interactive mode
void run_interpreter(spero::compiler::ReplDriver& repl) {
	using namespace spero;
	using namespace spero::parser;

	string::backing_type input;
	std::unordered_map<String, bool> flags{ { "compile", true }, { "interpret", true }, { "ast", true }, { "asm", true } };

	auto& state = repl.getState();
	GET_PERMISSIONS(state);
	state.files().push_back("");

	while (std::cout << "> " && getMultiline(std::cin, input)) {
		/**
		 * Handle repl-specific commands
		 *  :c - compile the given file (NOTE: currently not added to interpreter)
		 *  :l - load the given file into the interpreter context
		 *  :s - set repl flag (controlling what gets printed/etc)
		 *  :q - quit out of the repl
		 *
		 *  Defaults to receiving and running a multiline spero string
		 */
		if (auto command = input.substr(0, 2); command == ":q") {
			break;

		} else if (command == ":c") {
			state.files()[0] = input.substr(3);

			parser_mode = ParsingMode::FILE;	link = true;
			do_compile = true;				interpret = false;

		} else if (command == ":l") {
			state.files()[0] = input.substr(3);

			parser_mode = ParsingMode::FILE;
			do_compile = false;

		} else if (command == ":s") {
			for (auto flag : util::split(input.substr(3), ',')) {
				if (flag == "opt") {
					state.flipOptimization();
				} else {
					flags[flag] = !flags[flag];
				}
			}

			continue;

		} else {
			state.files()[0] = input;

			parser_mode = ParsingMode::STRING;	link = false;
			do_compile = flags["compile"];	interpret = flags["interpret"];
		}

		repl.interpret([&](auto&& ir) {
			if constexpr (std::is_same_v<std::decay_t<decltype(ir)>, parser::Stack>) {
				if (flags["ast"]) {
					printAST(std::cout << '\n', ir);
				}
			}

			if constexpr (std::is_same_v<std::decay_t<decltype(ir)>, std::unique_ptr<llvm::Module>>) {
				if (flags["asm"]) {
					printAssembly(std::cout, ir);
				}
			}
		});

		std::cout << std::endl;
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
		return compiler::CompilationDriver{ state }.compile();

	// Interactive mode
	} else {
		state.setPermissions(parser::ParsingMode::STRING, true, false, true);
		auto repl = compiler::ReplDriver{ state };

		return run_interpreter(repl), 0;
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

void printAssembly(std::ostream& s, std::unique_ptr<llvm::Module>& llvm_code) {
	llvm_code->print(llvm::outs(), nullptr);
	llvm::outs() << '\n';
}
