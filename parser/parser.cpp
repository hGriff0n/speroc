#include "parser.h"
#include "pegtl/analyze.hh"

namespace spero::parser {
	Stack parse(std::string input) {
		// Setup parsing state
		Stack s{};
		s.emplace_back(compiler::ast::Sentinel{});

		// Perform parse run
		pegtl::parse_string<grammar::program, actions::action>(input, "me", s);

		// Remove Sentinel nodes (currently only removes the first, any others are parser errors)
		s.pop_front();

		// Put an empty node on the stack if nothing was parsed (ie. empty input)
		if (s.size() == 0) s.emplace_back(std::make_unique<compiler::ast::Ast>());

		// Return completed ast (for now ast stack)
		return std::move(s);
	}

	size_t num_issues() {
		return pegtl::analyze<grammar::program>();
	}
}

namespace spero::util {
	std::string escape(std::string s) {
		return s;
	}
}