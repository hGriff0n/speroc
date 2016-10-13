#pragma once

#include "grammar.h"
#include "ast/ast.h"
#include "util/utils.h"
#include "util/stack_utils.h"
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
		 s.emplace_back(ast::Byte{ in.string(), 16 });
	});
	ACTION(bin_body, {
		s.emplace_back(ast::Byte{ in.string(), 2 });
	});
	ACTION(dec, {
		s.emplace_back(ast::Float{ in.string() });
	});
	ACTION(num, {
		s.emplace_back(ast::Int{ in.string() });
	});
	ACTION(str_body, {
		s.emplace_back(ast::String{ spero::util::escape(in.string()) });
	});
	ACTION(character, {
		s.emplace_back(ast::Char{ spero::util::escape(in.string())[1] });
	});
	ACTION(b_false, {
		s.emplace_back(ast::Bool{ false });
	});
	ACTION(b_true, {
		s.emplace_back(ast::Bool{ true });
	});
	ACTION(tuple, {
		// Find the location of the sentinel node (representing the '(' at the begining)
		auto sent = spero::util::findFirst<ast::Sentinel>(s.rbegin(), s.rend());

		// Move the set of values into the tuple (and create it on the stack)
		s.emplace_back(ast::Tuple{ sent.base(), s.end() });

		// Then remove the tuple values (and sentinel) from the stack
		s.erase((sent + 1).base(), s.end() - 1);
	});
	ACTION(array, {
		// Find the location of the sentinel node (representing the '(' at the begining)
		auto sent = spero::util::findFirst<ast::Sentinel>(s.rbegin(), s.rend());

		// Move the set of values into the tuple (and create it on the stack)
		s.emplace_back(ast::Array{ sent.base(), s.end() });

		// Then remove the tuple values (and sentinel) from the stack
		s.erase((sent + 1).base(), s.end() - 1);
	});

	// Bindings
	ACTION(name_path_part, { });
	ACTION(name_path, { s.pop_back(); });		// name_path is a bit too greedy in matching
	ACTION(var, { s.emplace_back(ast::BasicName{ in.string() }); });
	ACTION(typ, { s.emplace_back(ast::BasicName{ in.string() }); });
	ACTION(op, { s.emplace_back(ast::Operator{ in.string() }); });

	/*ACTION(program, {
		s.emplace_back(ast::Bool{ true });
	});*/

	// Atoms
	ACTION(scope, {
		// Find the location of the sentinel node (representing the '(' at the begining)
		auto sent = spero::util::findFirst<ast::Sentinel>(s.rbegin(), s.rend());

		// Move the set of values into the tuple (and create it on the stack)
		s.emplace_back(ast::Array{ sent.base(), s.end() });

		// Then remove the tuple values (and sentinel) from the stack
		s.erase((sent + 1).base(), s.end() - 1); 
	});

	// Atoms
}

#undef ACTION
#undef SENTINEL
