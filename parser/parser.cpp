#include "parser.h"

#include "pegtl/analyze.hh"

namespace spero::parser {
	Stack parse(std::string input) {
		Stack s{};
		s.emplace_back(compiler::ast::Sentinel{});

		pegtl::parse_string<grammar::program, actions::action>(input, "me", s);

		return s;
	}

	size_t num_issues() {
		return pegtl::analyze<grammar::program>();
	}
}