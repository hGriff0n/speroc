#pragma once

#include "grammar.h"
#include "ast/ast.h"
#include "util/string_utils.h"

#include <deque>
#include <algorithm>

#define ACTION(gram, app_def) \
template<> struct action<grammar::gram> { \
static void apply(const pegtl::action_input& in, Stack& s) app_def }

#define SENTINEL(gram) ACTION(gram, { s.emplace_back(ast::Sentinel{}); })

namespace spero::parser {
	using Stack = std::deque<spero::compiler::astnode>;
}

namespace spero::parser::actions {
	using namespace spero::compiler;

	template<class Rule>
	struct action : pegtl::nothing<Rule> {};

	// Sentinel Nodes
	SENTINEL(obrace);
	SENTINEL(obrack);
	SENTINEL(oparen);

	// Literals
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
	ACTION(b_false, {
		s.emplace_back(litnode{ ast::Bool{ false } });
	});
	ACTION(b_true, {
		s.emplace_back(litnode{ ast::Bool{ true } });
	});

	});
}

#undef ACTION#undef ACTION
#undef ACTION
#undef SENTINEL