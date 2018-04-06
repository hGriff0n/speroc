#include "compilation.h"
#include "parser/actions.h"
#include "codegen/AsmGenerator.h"

std::string spero::util::escape(std::string s) {
	return s;
}

namespace spero::compiler {
	using namespace parser;

	template<class Fn>
	Stack parse_impl(CompilationState& state, Fn&& parse) {
		// Setup parser state
		Stack ast;

		// Perform a parsing run, switching on whether the input is a file or not
		auto succ = parse(ast, state);
		if (!succ) {
			state.log(compiler::ID::err, "Error in parsing");
		}

		// Return the completed ast
		return std::move(ast);
	}

	Stack parse(std::string input, CompilationState& state) {
		using namespace spero::parser;

		return parse_impl(state, [&input](Stack& ast, CompilationState& state) {
			return tao::pegtl::parse<grammar::program, actions::action>(tao::pegtl::string_input<>{ input, "speroc:repl" }, ast, state);
		});
	}

	Stack parseFile(std::string file, CompilationState& state) {
		using namespace spero::parser;

		return parse_impl(state, [&file](Stack& ast, CompilationState& state) {
			return tao::pegtl::parse<grammar::program, actions::action>(tao::pegtl::file_input<>{ file }, ast, state);
		});
	}


	// Perform the various analysis stages
	MIR_t analyze(spero::parser::Stack s, CompilationState& state) {
		return std::move(s);
	}

	gen::Assembler backend(MIR_t& s, CompilationState& state) {
		gen::AsmGenerator visitor{ state };

		for (const auto& node : s) {
			node->accept(visitor);
		}

		return visitor.finalize();
	}

	// Perform the final compilation stages (produces direct assembly code)
	void codegen(gen::Assembler& emit, const std::string& in, const std::string& out, CompilationState& state, bool output_header) {
		// Open the output file
		std::ofstream o{ out };

		// Output file header information
		if (output_header) {
			o << ".text\n.intel_syntax noprefix\n.file \"" << in << "\"\n";
		}

		asmjit::StringBuilder sb;
		emit.dump(sb);
		o << sb.data() << '\n';

		// Output file footer information
		if (output_header) {
			//o << ".ident \"speroc\"\n";
		}
	}
}


// Because AsmJit somehow decides my computers architecture is 32-bit (x86) and I've currently hardcoded assembl generation,
// Since my installation of clang is 64-bit (x86-64), we have to force compilation under 32-bit mode
#define ASM_COMPILER "clang -masm=intel -m32"


// Full compilation implementation
bool spero::compile(spero::compiler::CompilationState& state, spero::parser::Stack& stack) {
	using namespace spero;

	// TODO: Initialize timing and other compilation logging structures
	// auto timer = state.getPhaseTimer();

	/*
	* Perform parsing and initial AST assembly
	*/
	// timer.start("Parsing");
	state.logTime();
	stack = compiler::parseFile(state.files()[0], state);
	state.logTime();
	// timer.end("Parsing");


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

