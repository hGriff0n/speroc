#pragma warning(disable : 4503)

#include <iostream>
#include <unordered_map>

#include "compilation.h"
#include "parser/base.h"
#include "interface/cmd_line.h"
#include "util/strings.h"

#include "codegen/LlvmIrGenerator.h"

template<class Stream>
Stream& getMultiline(Stream& in, spero::string::backing_type& s);
std::ostream& printAST(std::ostream& s, const spero::parser::Stack& stack);
void printAssembly(std::ostream&, std::unique_ptr<llvm::Module>& llvm_code);


// Helper function to run the interactive mode
void run_interpreter(cxxopts::Options& opts, spero::compiler::CompilationState& state, int& argc, char** argv) {
	using namespace spero;
	using namespace spero::parser;
	using compiler::ID;
	
	string::backing_type input;
	std::unordered_map<String, bool> flags{ { "compile", true }, { "interpret", true }, { "ast", true }, { "asm", true } };

	GET_PERMISSIONS(state);
	state.files().push_back("");

	llvm::Interpreter interpreter{ std::make_unique<llvm::Module>("speroc-repl", state.getContext()) };

	// Run the repl loop
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
				flags[flag] = !flags[flag];
			}

			continue;

		} else {
			state.files()[0] = input;

			parser_mode = ParsingMode::STRING;	link = false;
			do_compile = flags["compile"];	interpret = flags["interpret"];
		}


		// Run the input through the internal compilation loop (printing the ast and assembler)
		parser::Stack ast_stack;
		compile(state, ast_stack, &interpreter, [&](auto&& ir) {
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

void printAssembly(std::ostream& s, std::unique_ptr<llvm::Module>& llvm_code) {
	llvm_code->print(llvm::outs(), nullptr);
	llvm::outs() << '\n';
}


// Interpret the produced llvm ir code
void spero::compiler::interpret(llvm::Interpreter* inter, std::unique_ptr<llvm::Module> llvm_code, spero::compiler::CompilationState& state) {
	// Extract the "runtime" function and params from the module to ensure we run our expected code
	// NOTE: We currently only produce `jitfunc` to have type `() -> Int`
	auto jitfn = llvm_code->getFunction("jitfunc");

	// Add the next "layer" of interpreted code to the interpreter state
	// I think we still need to combine everything in a function though
	// And I'm not sure how this handles name clashes
	inter->addModule(std::move(llvm_code));

	if (jitfn) {

		// Call the "runtime" function
		std::vector<llvm::GenericValue> params;
		auto res = inter->runFunction(jitfn, params);

		// Print the output
		auto& output = llvm::outs() << "result: ";
#define TYPE(x) case llvm::Type::x##TyID
		switch (jitfn->getReturnType()->getTypeID()) {
			TYPE(Integer):
				output << res.IntVal << '\n';
				break;
			TYPE(Void):
			TYPE(Half):
			TYPE(Float):
			TYPE(Double):
			TYPE(X86_FP80):
			TYPE(FP128):
			TYPE(PPC_FP128):
			TYPE(Label):
			TYPE(Metadata):
			TYPE(X86_MMX):
			TYPE(Token):
			TYPE(Function):
			TYPE(Struct):
			TYPE(Array):
			TYPE(Pointer):
			TYPE(Vector):
			default:
				output << "Unimplemented type\n";
		}

		// Delete the `jit` function from the parent module so we always use the most recent one
		jitfn->eraseFromParent();
	}
}

void spero::compiler::transformAstForInterpretation(parser::Stack& ast_stack, CompilationState& state) {
	
}