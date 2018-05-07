#include "compilation.h"
#include "parser/actions.h"

#include "analysis/VarDeclPass.h"
#include "analysis/VarRefPass.h"
#include "codegen/AsmGenerator.h"

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
	MIR_t analyze(parser::Stack& ast_stack, CompilationState& state) {
		analysis::VarDeclPass decl_check{ state };
		ast::visit(decl_check, ast_stack);

		analysis::VarRefPass usage_check{ state, decl_check.finalize() };
		ast::visit(usage_check, ast_stack);

		auto table = usage_check.finalize();

		return std::move(table);
	}

	gen::Assembler backend(MIR_t globals, parser::Stack& ast_stack, CompilationState& state) {
		gen::AsmGenerator visitor{ std::move(globals), state };
		ast::visit(visitor, ast_stack);
		return visitor.finalize();
	}

	// Perform the final compilation stages (produces direct assembly code)
	void codegen(gen::Assembler& emit, const std::string& input_file, const std::string& output_file, CompilationState& state, bool output_header) {
		// Open the output file
		std::ofstream o{ output_file };

		// Output file header information
		if (output_header) {
			o << ".text\n.intel_syntax noprefix\n.file \"" << input_file << "\"\n";
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


// Because AsmJit somehow decides my computers architecture is 32-bit (x86) and I've currently hardcoded assembly generation,
// Since my installation of clang is 64-bit (x86-64), we have to force compilation under 32-bit mode
#define ASM_COMPILER "clang -masm=intel -m32"


// Full compilation implementation
bool spero::compile(spero::compiler::CompilationState& state, spero::parser::Stack& ast_stack) {
	using namespace spero;

	// TODO: Initialize timing and other compilation logging structures
	// auto timer = state.getPhaseTimer();

	/*
	* Perform parsing and initial AST assembly
	*/
	// timer.start("Parsing");
	state.logTime();
	ast_stack = compiler::parseFile(state.files()[0], state);
	state.logTime();
	// timer.end("Parsing");


	/*
	* Run through the analysis stages
	*
	* Don't mark this as a separate "phase" for timing purposes
	* The sub-phases perform their own timing passes
	*/
	auto table = (!state.failed())
		? compiler::analyze(ast_stack, state)
		: nullptr;


	/*
	* Run through the backend optimizations
	*
	* Don't mark this as a separate "phase" for timing purposes
	* The sub-phases perform their own timing passes
	*/
	auto asmCode = (!state.failed())
		? compiler::backend(std::move(table), ast_stack, state)
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

