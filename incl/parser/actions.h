#pragma once

#include "grammar.h"
#include "ast/ast.h"
#include "util/string_utils.h"
#include <vector>
#include <iostream>

#define ACTION(gram, app_def) \
template<> struct action<grammar::gram> { \
static void apply(const pegtl::action_input& in, Stack& s) app_def }

namespace spero::parser {
	using Stack = std::vector<spero::compiler::astnode>;
}

namespace spero::parser::actions {
	using namespace spero::compiler;

	template<class Rule>
	struct action : pegtl::nothing<Rule> {};

	ACTION(k_let, {

	});

	ACTION(hex_body, {
		 s.emplace_back(litnode{ ast::Byte{ in.string(), 16 } });
	});
	ACTION(bin_body, {
		s.emplace_back(litnode{ ast::Byte{ in.string(), 2 } });
	});
	ACTION(dec, {
		s.emplace_back(litnode{ ast::Float{ in.string() } });
	});
	ACTION(num, {
		s.emplace_back(litnode{ ast::Int{ in.string() } });
	});
	ACTION(str_body, {
		s.emplace_back(litnode{ ast::String{ spero::util::escape(in.string()) } });
	});
	ACTION(character, {
		s.emplace_back(litnode{ ast::Char{ spero::util::escape(in.string())[1] } });
	});
}

#undef ACTION