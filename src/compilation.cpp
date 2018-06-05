#include "compilation.h"
#include "parser/actions.h"

#include "analysis/VarDeclPass.h"
#include "analysis/VarRefPass.h"
#include "analysis/BasicTypingPass.h"

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
	MIR_t analyze(parser::Stack& ast_stack, CompilationState& state, analysis::AllTypes& type_list) {
		analysis::VarDeclPass decl_check{ state, type_list };
		ast::visit(decl_check, ast_stack);
		auto sym_table = decl_check.finalize();

		analysis::BasicTypingPass typing_check{ state, type_list, std::move(sym_table) };
		ast::visit(typing_check, ast_stack);
		sym_table = typing_check.finalize();

		analysis::VarRefPass usage_check{ state, type_list, std::move(sym_table) };
		ast::visit(usage_check, ast_stack);
		sym_table = usage_check.finalize();

		return std::move(sym_table);
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


// Full compilation implementation
bool spero::compile(spero::compiler::CompilationState& state, spero::parser::Stack& ast_stack) {
	return spero::compile(state, ast_stack, [](auto&&){});
}
