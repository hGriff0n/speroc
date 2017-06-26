#include "compiler.h"
#include "parser/control.h"
#include "pegtl/analyze.hh"
#include "codegen/AsmGenerator.h"

std::string spero::util::escape(std::string s) {
	return s;
}

namespace spero::compiler {
	using namespace parser;

	template<class Fn>
	std::tuple<size_t, Stack> parse_impl(Fn&& parse) {
		// Setup parser state
		Stack ast;
		ast.emplace_back(compiler::ast::Sentinel{});

		// Perform a parsing run, switching on whether the input is a file or not
		auto succ = parse(ast);

		// Maintain the ast stack invariants (no nullptr [at 0], non-empty)
		ast.pop_front();
		if (!ast.size()) {
			ast.emplace_back(std::make_unique<compiler::ast::Ast>(compiler::ast::Ast::Location{}));
		}

		// Return the completed ast
		return std::make_tuple(!succ, std::move(ast));
	}

	std::tuple<size_t, Stack> parse(std::string input, CompilationState& state) {
		return parse_impl([&input](Stack& ast) { return pegtl::parse_string<grammar::program, actions::action, control::control>(input, "me", ast); });
	}

	std::tuple<size_t, Stack> parseFile(std::string file, CompilationState& state) {
		return parse_impl([&file](Stack& ast) { return pegtl::parse_file<grammar::program, actions::action, control::control>(file, ast); });
	}


	// Perform the various analysis stages
	IR_t analyze(spero::parser::Stack s, CompilationState& state) {
		return std::move(s);
	}


	// Perform the final compilation stages (produces direct assembly code)
	void codegen(IR_t& s, std::string in, std::string out, CompilationState& state, bool output_header) {
		// Open the output file
		std::ofstream o{ out };

		// Output file header information
		if (output_header) {
			o << "\t.file \"" << in << "\"\n.text\n\t.p2align 4, 0x90\n";
		}

		auto visitor = spero::compiler::gen::AsmGenerator{ o, state };

		// Print everything directly to the file
		for (const auto& node : s) {
			node->visit(visitor);
		}

		o << '\n';
	}

}
