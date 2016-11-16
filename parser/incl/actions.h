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
	ACTION(tuple, {
		// stack: sentinel vals*
		std::deque<node> vals;
		while (util::at_node<ast::Ast>(s))
			vals.push_front(util::pop<ast::Ast>(s));

		s.pop_back();
		s.emplace_back(std::make_unique<ast::Tuple>(vals));
		// stack: tuple
	});
	ACTION(array, {
		// stack: sentinel vals*
		std::deque<node> vals;
		while (util::at_node<ast::Ast>(s))
			vals.push_front(util::pop<ast::Ast>(s));

		s.pop_back();
		s.emplace_back(std::make_unique<ast::Array>(vals));
	// stack: array
	});
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
	TOKEN(k_loop, ast::KeywordType::LOOP);
	TOKEN(k_wait, ast::KeywordType::WAIT);
	TOKEN(k_impl, ast::KeywordType::IMPL);

	// Bindings
	MK_NODE(var, std::make_unique<ast::BasicBinding>(in.string(), ast::BindingType::VARIABLE));
	MK_NODE(typ, std::make_unique<ast::BasicBinding>(in.string(), ast::BindingType::TYPE));
	MK_NODE(op, std::make_unique<ast::BasicBinding>(in.string(), ast::BindingType::OPERATOR));
	ACTION(name_path_part, {
		// stack: qual? basic
		auto part = util::pop<ast::BasicBinding>(s);

		// Apparently this can be triggered in block ??
		if (part) {
			if (util::at_node<ast::QualBinding>(s))
				util::view_as<ast::QualBinding>(s.back())->add(std::move(part));

			else
				s.emplace_back(std::make_unique<ast::QualBinding>(std::move(part)));
		}
		// stack: qual
	});
	INHERIT(name_path, name_path_part);

	// Atoms
	NONE(anon_type);
	ACTION(scope, {
		// stack: sentinel vals*
		std::deque<node> vals;
		while (util::at_node<ast::Ast>(s))
			vals.push_front(util::pop<ast::Ast>(s));

		s.pop_back();
		s.emplace_back(std::make_unique<ast::Block>(vals));
		// stack: block
	});
	ACTION(wait_stmt, {
		// stack: token expr
		auto expr = util::pop<ast::ValExpr>(s);
		s.pop_back();

		s.emplace_back(std::make_unique<ast::Wait>(std::move(expr)));
		// stack: wait
	});
	ACTION(fnseq, {
		// stack: expr, array?, anon_type?, tuple
		auto args = util::pop<ast::Tuple>(s);
		auto type = util::pop<ast::TypeExt>(s);
		auto inst = util::pop<ast::Array>(s);
		auto caller = util::pop<ast::Ast>(s);

		s.emplace_back(std::make_unique<ast::FnCall>(std::move(caller), std::move(type), std::move(args), std::move(inst)));
		// stack: fncall
	});
	ACTION(fneps, {
		// stack: expr | binding
		auto part = util::pop<ast::QualBinding>(s);
		if (part) s.emplace_back(std::make_unique<ast::FnCall>(std::move(part)));
		// TODO: Consider pushing a Variale node instead (can add the function consideration in later)
		// stack: fncall | expr
	});

	// Assignment
	ACTION(pat_var, {
		// stack: mut? var
		auto pat = std::make_unique<ast::PNamed>(std::move(util::pop<ast::BasicBinding>(s)));

		auto tkn = util::pop<ast::Token>(s);
		pat->is_mut = tkn && std::holds_alternative<ast::KeywordType>(tkn->val)
			&& std::get<ast::KeywordType>(tkn->val)._to_integral() == ast::KeywordType::MUT;

		s.push_back(std::move(pat));
		// stack: pattern
	});

	// Control
	ACTION(if_core, {
		// stack: token test body
		auto body = util::pop<ast::ValExpr>(s);
		auto test = util::pop<ast::ValExpr>(s);
		s.pop_back();
		s.emplace_back(std::make_unique<ast::Branch>(std::move(test), std::move(body)));
		// stack: branch
	});
	ACTION(if_dot_core, {
		// stack: body token test
		auto iter = std::rbegin(s);
		std::iter_swap(iter + 2, iter + 1);
		std::iter_swap(iter, iter + 1);
		action<grammar::if_core>::apply(in, s);
		// stack: branch
	});
	ACTION(elsif_rule, {
		// stack: branch token test body
		auto body = util::pop<ast::ValExpr>(s);
		auto test = util::pop<ast::ValExpr>(s);
		s.pop_back();
		util::view_as<ast::Branch>(s.back())->addIf(std::move(test), std::move(body));
		// stack: branch
	});
	ACTION(else_rule, {
		// stack: branch token body
		auto body = util::pop<ast::ValExpr>(s);
		s.pop_back();
		util::view_as<ast::Branch>(s.back())->addElse(std::move(body));
		// stack: branch
	});

	ACTION(loop, {
		// stack: token body
		auto part = util::pop<ast::ValExpr>(s);
		s.pop_back();
		s.emplace_back(std::make_unique<ast::Loop>(std::move(part)));
		// stack: loop
	});
	ACTION(dot_loop, {
		// stack: body token
		std::iter_swap(std::rbegin(s), std::rbegin(s) + 1);
		action<grammar::loop>::apply(in, s);
		// stack: loop
	});
	ACTION(while_l, {
		// stack: token test body
		auto body = util::pop<ast::ValExpr>(s);
		auto test = util::pop<ast::ValExpr>(s);
		s.pop_back();
		s.emplace_back(std::make_unique<ast::While>(std::move(test), std::move(body)));
		// stack: while
	});
	ACTION(dot_while, {
		// stack: body token test
		auto iter = std::rbegin(s);
		std::iter_swap(iter + 2, iter + 1);
		std::iter_swap(iter, iter + 1);
		action<grammar::while_l>::apply(in, s);
		// stack: while
	});
	ACTION(for_l, {
		// stack: token pattern generator body
		auto body = util::pop<ast::ValExpr>(s);
		auto gen = util::pop<ast::ValExpr>(s);
		auto pattern = util::pop<ast::Pattern>(s);
		s.pop_back();
		s.emplace_back(std::make_unique<ast::For>(std::move(pattern), std::move(gen), std::move(body)));
		// stack: for
	});
	ACTION(dot_for, {
		// stack: body token pattern generator
		auto iter = std::rbegin(s);
		std::iter_swap(iter + 3, iter + 2);
		std::iter_swap(iter + 1, iter + 2);
		std::iter_swap(iter + 1, iter);
		action<grammar::for_l>::apply(in, s);
		// stack: for
	});
	ACTION(jumps, {
		// stack: token expr?
		auto expr = util::pop<ast::ValExpr>(s);
		auto tkn = std::get<ast::KeywordType>(util::pop<ast::Token>(s)->val);

		switch (tkn) {
			case ast::KeywordType::BREAK:
				s.emplace_back(std::make_unique<ast::Break>(std::move(expr)));
				break;
			case ast::KeywordType::CONT:
				s.emplace_back(std::make_unique<ast::Continue>(std::move(expr)));
				break;
			case ast::KeywordType::YIELD:
				s.emplace_back(std::make_unique<ast::YieldExpr>(std::move(expr)));
				break;
			case ast::KeywordType::RET:
				s.emplace_back(std::make_unique<ast::Return>(std::move(expr)));
				break;
			default:
				break;
				//error
		}
		// stack: jump
	});
	ACTION(dot_jmp, {
		// stack: expr token
		std::iter_swap(std::rbegin(s), std::rbegin(s) + 1);
		action<grammar::jumps>::apply(in, s);
		// stack: jump
	});
	ACTION(case_stmt, {
		// stack: (sentinel | case) pattern* expr
		auto cas = std::make_unique<ast::Case>(std::move(util::pop<ast::ValExpr>(s)));

		std::deque<ptr<ast::Pattern>> pattern;
		while (util::at_node<ast::Pattern>(s))
			pattern.push_front(util::pop<ast::Pattern>(s));
		
		cas->setPattern(pattern);
		s.push_back(std::move(cas));
		// stack: (sentinel | case) case
	});
	ACTION(match_expr, {
		// stack: token expr sentinel case+
		std::deque<ptr<ast::Case>> cases;
		while (util::at_node<ast::Case>(s))
			cases.push_front(util::pop<ast::Case>(s));

		s.pop_back();

		// stack: token expr

		auto match_val = util::pop<ast::ValExpr>(s);
		s.pop_back();

		s.emplace_back(std::make_unique<ast::Match>(std::move(match_val), cases));
		// stack: match
	});
	ACTION(dot_match, {
		// stack: expr token sentinel case+
		std::deque<ptr<ast::Case>> cases;
		while (util::at_node<ast::Case>(s))
			cases.push_front(util::pop<ast::Case>(s));

		s.pop_back();

		// stack: expr token

		s.pop_back();
		s.emplace_back(std::make_unique<ast::Match>(util::pop<ast::ValExpr>(s), cases));
		// stack: match
	});

	// Placeholders
	NONE(placeholder);
	NONE(typ_gen_inst);
	NONE(typ_pointer);
	NONE(type);
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
	NONE(unary);

	// Expressions
	NONE(_index_);
	NONE(in_eps);
	NONE(range);
	NONE(impl_expr);
	ACTION(_binary_, {
		// stack: expr op expr
		std::iter_swap(std::rbegin(s) + 2, std::rbegin(s) + 1);

		// push arg tuple onto the stack
		std::deque<node> args;
		args.push_front(util::pop<ast::Ast>(s));
		args.push_front(util::pop<ast::Ast>(s));
		s.emplace_back(std::make_unique<ast::Tuple>(vals));

		// stack: op tuple
		action<grammar::fncall>::apply(in, s);
	});
	NONE(valexpr);
}

#undef SENTINEL
#undef PUSH
#undef NONE
#undef ACTION
