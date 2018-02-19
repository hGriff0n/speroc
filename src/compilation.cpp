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

		return visitor.get();
	}

	// Perform the final compilation stages (produces direct assembly code)
	void codegen(gen::Assembler& emit, const std::string& in, const std::string& out, CompilationState& state, bool output_header) {
		// Open the output file
		std::ofstream o{ out };

		// Output file header information
		if (output_header) {
			o << "\t.file \"" << in << "\"\n.text\n\t.p2align 4, 0x90\n";
		}

		asmjit::StringBuilder sb;
		emit.dump(sb);
		o << sb.data() << '\n';
	}

}
