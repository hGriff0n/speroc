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
	SENTINEL(anon_sep);


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
	template<> struct action<grammar::fn_rettype> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: type expr
			auto fn = std::make_unique<ast::FnBody>(util::pop<ast::ValExpr>(s), false);
			fn->ret = std::move(util::pop<ast::Type>(s));
			s.push_back(std::move(fn));
			// stack: fnbody
		}
	};
	ACTION(fn_forward, {
		// stack: expr
		s.emplace_back(std::make_unique<ast::FnBody>(util::pop<ast::ValExpr>(s), true));
		// stack: fnbody
	});
	ACTION(fn_def, {
		// stack: fnbody | expr
		if (!util::at_node<ast::FnBody>(s))
			s.emplace_back(std::make_unique<ast::FnBody>(util::pop<ast::ValExpr>(s), false));
		// stack: fnbody
	});
	ACTION(fn_or_tuple, {
		// stack: tuple? fnbody
		auto fn = util::pop<ast::FnBody>(s);
		if (fn) {
			fn->args = std::move(util::pop<ast::Tuple>(s));
			s.push_back(std::move(fn));
		}
		// stack: tuple | fnbody
	});


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
	ACTION(typ_pointer, {
		if (in.string() == "&") s.emplace_back(std::make_unique<ast::Token>(ast::PtrStyling::POINTER));
		else if (in.string() == "*") s.emplace_back(std::make_unique<ast::Token>(ast::PtrStyling::VIEW));
		else s.emplace_back(std::make_unique<ast::Token>(ast::PtrStyling::NA));
	});


	// Atoms
	ACTION(anon_type, {
		// stack: sentinel tuple? scope
		auto scp = util::pop<ast::Block>(s);
		auto tup = util::pop<ast::Tuple>(s);
		s.pop_back();
		s.emplace_back(std::make_unique<ast::TypeExt>(std::move(tup), ast::move(scp)));
		// stack: anon
	});
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


	// Decorators
	ACTION(type, {
		// stack: qualname array? ptr
		auto tkn = util::pop<ast::Token>(s);
		auto inst = util::pop<ast::Array>(s);
		auto name = util::pop<ast::QualBinding>(s);

		s.emplace_back(std::make_unique<ast::InstType>(std::move(name), std::move(inst), std::get<ast::PtrStyling>(tkn->val)));
	});
	ACTION(unary, {
		if (in.string() == "&") s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::DEREF));
		else if (in.string() == "!") s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::NOT));
		else if (in.string() == "-") s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::NA));
		else s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::NA));
	});
	TOKEN(anot_glob, ast::UnaryType::NOT);
	ACTION(annotation, {
		// stack: name glob? tuple?
		auto tup = util::pop<ast::Tuple>(s);
		auto glob = util::pop<ast::Token>(s);
		auto name = util::pop<ast::BasicBinding>(s);
		s.emplace_back(std::make_unique<ast::Annotation>(std::move(name), std::move(tup), glob));
		// stack: annotation
	});
	ACTION(mod_dec, {
		// stack: KWD var*
		std::deque<ptr<ast::BasicBinding>> mod;
		while (util::at_node<ast::BasicBinding>(s))
			mod.push_front(util::pop<ast::BasicBinding>(s));

		s.pop_back();
		s.emplace_back(std::make_unique<ast::ModDec>(std::make_unique<ast::QualBinding>(mod)));
		// stack: mod_dec
	});
	ACTION(mut_type, {
		// stack: mut? type
		auto type = util::pop<ast::Type>(s);

		auto tkn = util::pop<ast::Token>(s);
		if (tkn)
			type->is_mut = std::holds_alternative<ast::KeywordType>(tkn->val)
				&& std::get<ast::KeywordType>(tkn->val)._to_integral() == ast::KeywordType::MUT;

		s.push_back(std::move(type));
		// stack: type
	});
	ACTION(type_tuple, {
		// stack: sentinel type*
		std::deque<ptr<ast::Type>> types;
		while (util::at_node<ast::Type>(s))
			types.push_front(util::pop<ast::Type>(s));

		s.pop_back();
		s.emplace_back(std::make_unique<ast::TupleType>(types));
		// stack: tupletype
	});
	ACTION(inf, {
		// stack: tuple? type
		auto type = util::pop<ast::Type>(s);
		if (util::view_as<ast::TupleType>(s))
			s.emplace_back(std::make_unique<ast::FuncType>(util::pop<ast::TupleType>(s), std::move(type)));

		else
			s.push_back(std::move(type));
		// stack: type
	});
	ACTION(gen_variance, {
		if (in.string() == "+") s.emplace_back(std::make_unique<ast::Token>(ast::VarianceType::COVARIANT));
		else if (in.string() == "-") s.emplace_back(std::make_unique<ast::Token>(ast::VarianceType::CONTRAVARIANT));
		else s.emplace_back(std::make_unique<ast::Token>(ast::VarianceType::NA));
	});
	ACTION(gen_subrel, {
		if (in.string() == "::") s.emplace_back(std::make_unique<ast::Token>(ast::RelationType::IMPLS));
		else if (in.string() == "!:") s.emplace_back(std::make_unique<ast::Token>(ast::RelationType::NOT_IMPLS));
		else if (in.string() == ">")  s.emplace_back(std::make_unique<ast::Token>(ast::RelationType::SUPERTYPE));
		else  s.emplace_back(std::make_unique<ast::Token>(ast::RelationType::SUBTYPE));
	});
	ACTION(gen_type, {
		// stack: typ variance? relation? type?
		auto type = util::pop<ast::Type>(s);
		auto rel = util::pop<ast::Token>(s);
		auto var = util::pop<ast::Token>(s);
		auto name = util::pop<ast::BasicBinding>(s);

		s.emplace_back(std::make_unique<ast::TypeGeneric>(
			std::move(name), std::move(type),
			rel ? std::get<ast::RelationType>(rel->val) : ast::RelationType::NA,
			var ? std::get<ast::VarianceType>(rel->val) : ast::VarianceType::NA
		));
		// stack: generic
	});
	ACTION(gen_val, {
		// stack: name type? expr?
		auto expr = util::pop<ast::ValExpr>(s);
		auto type = util::pop<ast::Type>(s);
		auto name = util::pop<ast::BasicBinding>(s);
		s.emplace_back(std::make_unique<ast::ValueGeneric>(std::move(name), std::move(type), std::move(expr)));
		// stack: generic
	});
	MK_NODE(use_any, std::make_unique<ast::ImportPiece>());
	ACTION(use_one, {
		// stack: var
		s.emplace_back(std::make_unique<ast::ImportName>(util::pop<ast::BasicBinding>(s)));
		// stack: ImportName
	});
	ACTION(use_many, {
		// stack: sentinel var*
		std::deque<ptr<ast::ImportName>> imps;
		while (util::at_node<ast::BasicBinding>(s))
			imps.push_front(std::make_node<ast::ImportName>(util::pop<ast::BasicBinding>(s)));
	
		s.pop_back();
		s.emplace_back(std::make_unique<ast::ImportGroup>(imps));
		// stack: ImportGroup
	});
	ACTION(use_path, {
		// stack: MOD imps*
		std::deque<ptr<ast::ImportPiece>> imps;
		while (util::at_node<ast::ImportPiece>(s))
			imps.push_front(util::pop<ast::ImportPiece>(s));

		s.emplace_back(std::make_unique<ast::ImportPart>(imps));
		// stack: imp_part
	});
	ACTION(use_elem, {
		// stack: (var var?) | (typ typ?)
		auto bind1 = util::pop<ast::BasicBinding>(s);
		auto bind2 = util::pop<ast::BasicBinding>(s);
		if (bind2)
			s.emplace_back(std::make_unique<ast::ImportName>(std::move(bind2), std::move(bind1)));
		else
			s.emplace_back(std::make_unique<ast::ImportName>(std::move(bind1)));
		// stack: ImportName
	});


	// Assignment
	ACTION(var_tuple, {
		// stack: sentinel AssignPattern*
		std::deque<ptr<ast::AssignPattern>> pat;
		while (util::at_node<ast::AssignPattern>(s))
			pat.push_front(util::pop<ast::AssignPattern>(s));

		s.pop_back();
		s.emplace_back(std::make_unique<ast::AssignTuple>(pat));
		// stack: AssignTuple
	});
	ACTION(var_pattern, {
		// stack: binding
		if (util::at_node<ast::BasicBinding>(s))
			s.emplace_back(std::make_unique<ast::AssignName>(util::pop<ast::BasicBinding>(s)));
		// stack: VarPat
	});
	INHERIT(var_type, var_pattern);
	ACTION(tuple_pat, {
		// stack: sentinel pattern*
		std::deque<ptr<ast::Pattern>> vals;
		while (util::at_node<ast::Pattern>(s))
			vals.push_front(util::pop<ast::Pattern>(s));

		s.pop_back();
		s.emplace_back(std::make_unique<ast::PTuple>(vals));
		// stack: PTuple
	});
	ACTION(pat_tuple, {
		// stack: mut? PTuple
		std::iter_swap(std::rbegin(s) + 1, std::rbegin(s));
		
		auto tkn = util::pop<ast::Token>(s);
		if (tkn)
			util::view_as<ast::Pattern>(s.back())->is_mut = std::holds_alternative<ast::KeywordType>(tkn->val)
				&& std::get<ast::KeywordType>(tkn->val)._to_integral() == ast::KeywordType::MUT;

		else
			std::iter_swap(std::rbegin(s) + 1, std::rbegin(s));
		// stack: PTuple
	});
	ACTION(pat_adt, {
		//stack: binding PTuple?
		auto args = util::pop<ast::PTuple>(s);
		auto name = util::pop<ast::BasicBinding>(s);
		s.emplace_back(std::make_unique<ast::PAdt>(std::move(name), std::move(args)));
		//stack: PAdt
	});
	ACTION(pat_var, {
		// stack: mut? var
		auto pat = std::make_unique<ast::PNamed>(std::move(util::pop<ast::BasicBinding>(s)));

		auto tkn = util::pop<ast::Token>(s);
		pat->is_mut = tkn && std::holds_alternative<ast::KeywordType>(tkn->val)
			&& std::get<ast::KeywordType>(tkn->val)._to_integral() == ast::KeywordType::MUT;

		s.push_back(std::move(pat));
		// stack: pattern
	});
	MK_NODE(pat_any, std::make_unique<ast::Pattern>());
	ACTION(var_assign, {
		// stack: pattern generic? (inf expr?)|(inf? expr)
		auto val = util::pop<ast::ValExpr>(s);
		auto inf = util::pop<ast::Type>(s);

		// Get the generics list
		GenArray gen;
		while (util::at_node<ast::GenericPart>(s))
			gen.push_front(util::pop<ast::GenericPart>(s));

		auto bind = util::pop<ast::AssignPattern>(s);

		if (val)
			s.emplace_back(std::make_unique<ast::VarAssign>(std::move(bind), gen, std::move(val), std::move(inf)));
		else
			s.emplace_back(std::make_unique<ast::Interface>(std::move(bind), gen, std::move(inf)));
		// stack: VarAssign
	});
	template<> struct action<grammar::adt_con> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: binding TupleType?
			auto args = util::pop<ast::TupleType>(s);
			auto name = util::pop<ast::BasicBinding>(s);
			s.emplace_back(std::make_unique<ast::Adt>(std::move(name), std::move(args)));
			// stack: Adt
		}
	};
	ACTION(rhs_inf, {
		// stack: binding
		s.emplace_back(util::pop<ast::BasicBinding>(s));
		// stack: type
	});
	template<> struct action<grammar::type_assign> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: binding generics* inf? (adt|tuple)* scope
			auto body = util::pop<ast::Block>(s);

			// Get the constructor list
			std::deque<node> cons;
			while (util::at_node<ast::Adt>(s) || util::at_node<ast::Tuple>(s))
				cons.push_front(util::pop<ast::Ast>(s));

			auto inherit = util::pop<ast::Type>(s);

			// Get the generics list
			GenArray gen;
			while (util::at_node<ast::GenericPart>(s))
				gen.push_front(util::pop<ast::GenericPart>(s));

			auto bind = util::pop<ast::AssignPattern>(s);
			s.emplace_back(std::make_unique<ast::TypeAssign>(std::move(bind), cons, gen, std::move(body), std::move(inherit)));
			// stack: TypeAssign
		}
	};
	ACTION(assign, {
		// stack: visibility interface
		auto assign = util::pop<ast::Interface>(s);
		auto tkn = util::pop<ast::Token>(s);

		if (tkn && std::holds_alternative<ast::VisibilityType>(tkn->val))
			assign->setVisibility(std::get<ast::VisibilityType>(tkn->val));
		else {
		}

		s.push_back(std::move(assign));
		// stack: interface
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


	// Expressions
	ACTION(_index_, {
		// stack: (expr|index) expr
		auto expr = util::pop<ast::ValExpr>(s);
		if (util::at_node<ast::Index>(s))
			util::view_as<ast::Index>(s)->add(std::move(expr));
		else {
			auto expr2 = util::pop<ast::ValExpr>(s);
			s.emplace_back(std::make_unique<ast::Index>(std::move(expr), std::move(expr2)));
		}
		// stack: index
	});
	ACTION(in_eps, {
		// stack: index, inf?
		auto typ = util::pop<ast::Type>(s);
		util::view_as<ast::Index>(s)->inf = std::move(typ);
		// stack: index
	});
	ACTION(range, {
		// stack: expr, expr
		auto stop = util::pop<ast::ValExpr>(s);
		auto start = util::pop<ast::ValExpr>(s);
		s.emplace_back(std::make_unique<ast::Range>(std::move(start), std::move(stop)));
		// stack: range
	});
	ACTION(impl_expr, {
		// stack: IMPL qual
		auto qual = util::pop<ast::QualBinding>(s);
		s.pop_back();
		s.emplace_back(std::make_unique<ast::ImplExpr>(std::move(qual)));
		// stack: impl
	});
	ACTION(_binary_, {
		// stack: expr op expr
		std::iter_swap(std::rbegin(s) + 2, std::rbegin(s) + 1);

		// push arg tuple onto the stack
		std::deque<node> args;
		args.push_front(util::pop<ast::Ast>(s));
		args.push_front(util::pop<ast::Ast>(s));
		s.emplace_back(std::make_unique<ast::Tuple>(args));

		// stack: op tuple
		action<grammar::fnseq>::apply(in, s);
	});
	ACTION(valexpr, {
		// stack: mut? expr
		std::iter_swap(std::rbegin(s) + 1, std::rbegin(s));

		auto tkn = util::pop<ast::Token>(s);
		if (tkn)
			util::view_as<ast::ValExpr>(s.back())->is_mut = std::holds_alternative<ast::KeywordType>(tkn->val)
				&& std::get<ast::KeywordType>(tkn->val)._to_integral() == ast::KeywordType::MUT;

		else
			std::iter_swap(std::rbegin(s) + 1, std::rbegin(s));
		// stack: expr
	});
}

#undef SENTINEL
#undef PUSH
#undef NONE
#undef ACTION
