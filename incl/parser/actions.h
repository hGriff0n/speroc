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

#define PUSH(gram, val) ACTION(gram, { s.emplace_back((val)); })
#define SENTINEL(gram) PUSH(gram, ast::Sentinel{});

namespace spero::parser {
	using Stack = std::deque<spero::compiler::StackVals>;
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
	PUSH(hex_body, std::make_unique<ast::Byte>(in.string(), 16));
	PUSH(bin_body, std::make_unique<ast::Byte>(in.string(), 2));
	PUSH(integer, std::make_unique<ast::Int>(in.string()));
	PUSH(dec, std::make_unique<ast::Float>(in.string()));
	PUSH(str_body, std::make_unique<ast::String>(spero::util::escape(in.string())));
	PUSH(char_body, std::make_unique<ast::Char>(spero::util::escape(in.string())[0]));
	PUSH(b_false, std::make_unique<ast::Bool>(false));
	PUSH(b_true, std::make_unique<ast::Bool>(true));

//	ACTION(tuple, {
//		// Find the location of the sentinel node (representing the '(' at the begining)
//		auto sent = spero::util::findFirst<connode, ast::Sentinel>(s.rbegin(), s.rend());
//
//		// Move the set of values into the tuple (and create it on the stack)
//		s.emplace_back(ast::Tuple{ sent.base(), s.end() });
//
//		// Then remove the tuple values (and sentinel) from the stack
//		s.erase((sent + 1).base(), s.end() - 1);
//	});
//	ACTION(array, {
//		// Find the location of the sentinel node (representing the '(' at the begining)
//		auto sent = spero::util::findFirst<connode, ast::Sentinel>(s.rbegin(), s.rend());
//
//		// Move the set of values into the tuple (and create it on the stack)
//		s.emplace_back(ast::Array{ sent.base(), s.end() });
//
//		// Then remove the tuple values (and sentinel) from the stack
//		s.erase((sent + 1).base(), s.end() - 1);
//	});
//
//	// Bindings
//	ACTION(name_path_part, {
//		if (util::at_top<connode, ast::NamePath>(s))
//			s.back().add(in.string());
//
//		else
//			s.emplace_back(ast::NamePath{ in.string() });
//	});
//	ACTION(name_path, { s.pop_back(); });		// name_path is a bit too greedy in matching
//	ACTION(var, { s.emplace_back(ast::BasicName{ in.string() }); });
//	ACTION(typ, { s.emplace_back(ast::BasicName{ in.string() }); });
//	ACTION(op, { s.emplace_back(ast::Operator{ in.string() }); });
//
//	/*ACTION(program, {
//		s.emplace_back(ast::Bool{ true });
//	});*/
//
//	// Atoms
//	ACTION(scope, {
//		// Find the location of the sentinel node (representing the '(' at the begining)
//		auto sent = spero::util::findFirst<connode, ast::Sentinel>(s.rbegin(), s.rend());
//
//		// Move the set of values into the tuple (and create it on the stack)
//		s.emplace_back(ast::Array{ sent.base(), s.end() });
//
//		// Then remove the tuple values (and sentinel) from the stack
//		s.erase((sent + 1).base(), s.end() - 1); 
//	});
//
//	// Atoms
}

#undef ACTION
#undef SENTINEL
