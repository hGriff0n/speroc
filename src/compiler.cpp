#include "compiler.h"
#include "parser/control.h"
#include "pegtl/analyze.hh"
#include "codegen/AsmGenerator.h"

namespace spero::parser {
	std::ostream& printAST(std::ostream& s, const Stack& stack) {
		for (const auto& node : stack)
			if (node) node->prettyPrint(s, 0) << '\n';
			else s << "nullptr\n";

			return s << '\n';
	}
}

namespace spero::util {
	std::string escape(std::string s) {
		return s;
	}
}

namespace spero::compiler {
	using namespace parser;

	std::tuple<bool, Stack> parse_impl(std::string input, bool is_filename) {
		// Setup parser state
		Stack ast;
		ast.emplace_back(compiler::ast::Sentinel{});

		// TODO: Report starting parsing run

		// Perform a parsing run, switching on whether the input is a file or not
		auto succ = is_filename
			? pegtl::parse_file<grammar::program, actions::action, control::control>(input, ast)
			: pegtl::parse_string<grammar::program, actions::action, control::control>(input, "me", ast);

		// TODO: Report ending parsing run

		// Maintain the ast stack invariants (no nullptr [at 0], non-empty)
		ast.pop_front();
		if (!ast.size()) ast.emplace_back(std::make_unique<compiler::ast::Ast>(compiler::ast::Ast::Location{}));

		// Return the completed ast
		return std::make_tuple(succ, std::move(ast));
	}

	std::tuple<bool, Stack> parse(std::string input) {
		return parse_impl(input, false);
	}

	std::tuple<bool, Stack> parseFile(std::string file) {
		return parse_impl(file, true);
	}

	// Perform the various analysis stages
	IR_t analyze(spero::parser::Stack& s) {
		return std::move(s);
	}

	// Perform the final compilation stages (produces direct assembly code)
	void codegen(IR_t& s, std::string in, std::string out, std::ostream& debug) {
		// debug << "Starting compilation phase...\n";

		// Open the output file
		std::ofstream o{ out };
		// debug << "Opened file " << out << " for compilation output\n";

		// Output file header information
		o << "\t.file \"" << in << "\"\n.text\n";

		auto visitor = spero::compiler::gen::AsmGenerator{ o };

		// Print everything directly to the file
		for (const auto& node : s)
			node->visit(visitor);

		o << '\n';

		// End the compilation phase
		// debug << "Ending compilation phase...\n";
	}

}