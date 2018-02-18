#include "compilation.h"
#include "parser/actions.h"
#include "codegen/AsmGenerator.h"

std::string spero::util::escape(std::string s) {
	return s;
}

namespace spero::compiler {
	using namespace parser;

	// TODO: Fix the semantic switching of 'succ'
	template<class Fn>
	std::tuple<size_t, Stack> parse_impl(Fn&& parse) {
		// Setup parser state
		Stack ast;

		// Perform a parsing run, switching on whether the input is a file or not
		auto succ = parse(ast);

		// Return the completed ast
		return std::make_tuple(!succ, std::move(ast));
	}

	std::tuple<size_t, Stack> parse(std::string input, CompilationState& state) {
		using namespace spero::parser;

		return parse_impl([&input, &state](Stack& ast) {
			return tao::pegtl::parse<grammar::program, actions::action>(tao::pegtl::string_input<>{ input, "speroc:repl" }, ast, state);
		});
	}

	std::tuple<size_t, Stack> parseFile(std::string file, CompilationState& state) {
		using namespace spero::parser;

		return parse_impl([&file, &state](Stack& ast) {
			return tao::pegtl::parse<grammar::program, actions::action>(tao::pegtl::file_input<>{ file }, ast, state);
		});
	}


	// Perform the various analysis stages
	IR_t analyze(spero::parser::Stack s, CompilationState& state) {
		return std::move(s);
	}

	gen::Assembler backend(IR_t& s, CompilationState& state) {
		gen::AsmGenerator visitor{ state };

		for (const auto& node : s) {
			node->accept(visitor);
		}

		return visitor.result();
	}

	/*
	Proposed Interface Change:

	void interpret(asmjit::x86::Builder& emit, CompilationState& state) {
		
	}

	void codegen(asmjit::x86::Builder& emit, const std::string& in, const std::string& out, CompilationState& state, bool output_header) {
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

	spero::compiler::gen::AsmGenerator{ state };
	for (const auto& node : s) {

	*/

	// Perform the final compilation stages (produces direct assembly code)
	void codegen(IR_t& s, const std::string& in, const std::string& out, CompilationState& state, bool output_header) {
		// Produce the assembly code
		asmjit::StringBuilder sb;
		auto asmCode = backend(s, state);
		asmCode.dump(sb);

		// Open the output file
		std::ofstream o{ out };

		// Output file header information
		if (output_header) {
			o << "\t.file \"" << in << "\"\n.text\n\t.p2align 4, 0x90\n";
		}

		// Write everything to the file
		o << sb.data() << '\n';
	}

}
