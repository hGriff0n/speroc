#pragma once

#include "grammar.h"
#include "ast.h"
#include "utils.h"

#include <deque>
#include <algorithm>
#include <iostream>

// Baseline Definitions
#define ACTION(gram, app_def) \
template<> struct action<grammar::gram> { \
static void apply(const pegtl::action_input& in, Stack& s) app_def }

#define INHERIT(gram, base) \
template<> struct action<grammar::gram> : action<grammar::base> {}

// Specialized Definitions for Common Cases
#define MK_NODE(gram, val) ACTION(gram, { s.emplace_back((val)); })
#define PUSH(gram, node, val) MK_NODE(gram, std::make_unique<node>(val))
#define SENTINEL(gram) MK_NODE(gram, ast::Sentinel{});
#define TOKEN(gram, val) PUSH(gram, ast::Token, val)
#define NONE(gram) ACTION(gram, {})

namespace spero::parser {
	using Stack = std::deque<spero::compiler::node>;
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
	MK_NODE(hex_body, std::make_unique<ast::Byte>(in.string(), 16));
	MK_NODE(bin_body, std::make_unique<ast::Byte>(in.string(), 2));
	PUSH(integer, ast::Int, in.string());
	PUSH(dec, ast::Float, in.string());
	PUSH(str_body, ast::String, spero::util::escape(in.string()));
	PUSH(char_body, ast::Char, spero::util::escape(in.string())[0]);
	PUSH(b_false, ast::Bool, false);
	PUSH(b_true, ast::Bool, true);
	NONE(tuple);
	NONE(array);
	NONE(fn_rettype);
	NONE(fn_forward);
	NONE(fn_def);
	NONE(fn_or_tuple);

	// Keywords
	TOKEN(k_let, ast::VisibilityType::PROTECTED);
	TOKEN(k_def, ast::VisibilityType::PUBLIC);
	TOKEN(k_static, ast::VisibilityType::STATIC);
	TOKEN(k_mut, ast::KeywordType::MUT);
	TOKEN(k_mod, ast::KeywordType::MOD);
	TOKEN(k_use, ast::KeywordType::USE);
	TOKEN(k_match, ast::KeywordType::MATCH);
	TOKEN(k_if, ast::KeywordType::IF);
	TOKEN(k_elsif, ast::KeywordType::ELSIF);
	TOKEN(k_else, ast::KeywordType::ELSE);
	TOKEN(k_while, ast::KeywordType::WHILE);
	TOKEN(k_for, ast::KeywordType::FOR);
	TOKEN(k_break, ast::KeywordType::BREAK);
	TOKEN(k_continue, ast::KeywordType::CONT);
	TOKEN(k_yield, ast::KeywordType::YIELD);
	TOKEN(k_ret, ast::KeywordType::RET);
	TOKEN(k_wait, ast::KeywordType::WAIT);
	TOKEN(k_impl, ast::KeywordType::IMPL);

	// Bindings
	MK_NODE(var, std::make_unique<ast::BasicBinding>(in.string(), ast::BindingType::VARIABLE));
	MK_NODE(typ, std::make_unique<ast::BasicBinding>(in.string(), ast::BindingType::TYPE));
	MK_NODE(op, std::make_unique<ast::BasicBinding>(in.string(), ast::BindingType::OPERATOR));
	ACTION(name_path_part, {
		auto part = util::pop<ast::BasicBinding>(s);

		if (part) {
			if (util::is_type<ast::QualBinding>(s.back()))
				dynamic_cast<ast::QualBinding*>(s.back().get())->add(std::move(part));

			else
				s.emplace_back(std::make_unique<ast::QualBinding>(std::move(part)));
		} else {
			// error
		}
	});
	INHERIT(name_path, name_path_part);

	// Atoms
	NONE(anon_type);
	NONE(scope);
	NONE(wait_stmt);
	// Placeholders
	NONE(placeholder);
	NONE(typ_gen_inst);
	NONE(typ_pointer);
	NONE(type);
	NONE(fncall);
	NONE(annotation);
	NONE(mod_dec);
	NONE(mut_type);
	NONE(type_tuple);
	NONE(inf_fn_args);
	NONE(inf);
	NONE(gen_variance);
	NONE(gen_subtype);
	NONE(gen_type);
	NONE(gen_val);
	NONE(generic);
	NONE(use_path_elem);
	NONE(use_path);
	NONE(use_elem);
	NONE(adt_con);
	NONE(var_tuple);
	NONE(var_pattern);
	NONE(var_assign);
	NONE(type_assign);
	NONE(assign);
	NONE(pat_tuple);
	NONE(pattern);
	NONE(branch);
	NONE(dot_if);
	NONE(loop);
	NONE(while_core);
	NONE(while_l);
	NONE(for_core);
	NONE(for_l);
	NONE(jumps);
	NONE(case_stmt);
	NONE(match_expr);
	NONE(_index_);
	NONE(in_eps);
	NONE(in_ctrl);
	NONE(unary);
	NONE(index);
	NONE(range);
	NONE(binary);
	NONE(valexpr);
	NONE(impl_expr);
}

#undef SENTINEL
#undef PUSH
#undef NONE
#undef ACTION
