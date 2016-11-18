#pragma once

#include "grammar.h"
#include "ast.h"
#include "utils.h"

#include <deque>
#include <algorithm>
#include <iostream>


// Specialized Definitions for Common Cases
#define SENTINEL(gram) \
template<> struct action<grammar::gram> {\
	static void apply(const pegtl::action_input& in, Stack& s) { \
	s.emplace_back(ast::Sentinel{}); }}
#define TOKEN(gram, val) \
template<> struct action<grammar::gram> {\
	static void apply(const pegtl::action_input& in, Stack& s) { \
	s.emplace_back(std::make_unique<ast::Token>(val)); }}
#define INHERIT(gram, base) \
template<> struct action<grammar::gram> : action<grammar::base> {}

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
	template<> struct action<grammar::hex_body> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::Byte>(in.string(), 16));
		}
	};
	template<> struct action<grammar::bin_body> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::Byte>(in.string(), 2));
		}
	};
	template<> struct action<grammar::integer> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::Int>(in.string()));
		}
	};
	template<> struct action<grammar::dec> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::Float>(in.string()));
		}
	};
	template<> struct action<grammar::str_body> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::String>(util::escape(in.string())));
		}
	};
	template<> struct action<grammar::char_body> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::Char>(util::escape(in.string())[0]));
		}
	};
	template<> struct action<grammar::b_false> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::Bool>(false));
		}
	};
	template<> struct action<grammar::b_true> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::Bool>(true));
		}
	};
	template<> struct action<grammar::tuple> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: sentinel vals*
			std::deque<node> vals;
			while (util::at_node<ast::Ast>(s))
				vals.push_front(util::pop<ast::Ast>(s));

			s.pop_back();
			s.emplace_back(std::make_unique<ast::Tuple>(vals));
			// stack: tuple
		}
	};
	template<> struct action<grammar::array> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: sentinel vals*
			std::deque<node> vals;
			while (util::at_node<ast::Ast>(s))
				vals.push_front(util::pop<ast::Ast>(s));

			s.pop_back();
			s.emplace_back(std::make_unique<ast::Array>(vals));
			// stack: array
		}
	};
	template<> struct action<grammar::fn_rettype> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: type expr
			auto fn = std::make_unique<ast::FnBody>(util::pop<ast::ValExpr>(s), false);
			fn->ret = std::move(util::pop<ast::Type>(s));
			s.push_back(std::move(fn));
			// stack: fnbody
		}
	};
	template<> struct action<grammar::fn_forward> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: expr
			s.emplace_back(std::make_unique<ast::FnBody>(util::pop<ast::ValExpr>(s), true));
			// stack: fnbody
		}
	};
	template<> struct action<grammar::fn_def> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: fnbody | expr
			if (!util::at_node<ast::FnBody>(s))
				s.emplace_back(std::make_unique<ast::FnBody>(util::pop<ast::ValExpr>(s), false));
			// stack: fnbody
		}
	};
	template<> struct action<grammar::fn_or_tuple> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: tuple? fnbody
			auto fn = util::pop<ast::FnBody>(s);
			if (fn) {
				fn->args = std::move(util::pop<ast::Tuple>(s));
				s.push_back(std::move(fn));
			}
			// stack: tuple | fnbody
		}
	};


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
	template<> struct action<grammar::var> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::BasicBinding>(in.string(), ast::BindingType::VARIABLE));
		}
	};
	template<> struct action<grammar::typ> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::BasicBinding>(in.string(), ast::BindingType::TYPE));
		}
	};
	template<> struct action<grammar::op> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::QualBinding>(
				std::make_unique<ast::BasicBinding>(in.string(), ast::BindingType::OPERATOR)
			));
		}
	};
	template<> struct action<grammar::name_path_part> {
		static void apply(const pegtl::action_input& in, Stack& s) {
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
		}
	};
	INHERIT(name_path, name_path_part);
	template<> struct action<grammar::typ_pointer> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			if (in.string() == "&") s.emplace_back(std::make_unique<ast::Token>(ast::PtrStyling::POINTER));
			else if (in.string() == "*") s.emplace_back(std::make_unique<ast::Token>(ast::PtrStyling::VIEW));
			else s.emplace_back(std::make_unique<ast::Token>(ast::PtrStyling::NA));
		}
	};


	// Atoms
	template<> struct action<grammar::anon_type> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: sentinel tuple? scope
			auto scp = util::pop<ast::Block>(s);
			auto tup = util::pop<ast::Tuple>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::TypeExt>(std::move(tup), std::move(scp)));
			// stack: anon
		}
	};
	template<> struct action<grammar::scope> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: sentinel vals*
			std::deque<node> vals;
			while (util::at_node<ast::Ast>(s))
				vals.push_front(util::pop<ast::Ast>(s));

			s.pop_back();
			s.emplace_back(std::make_unique<ast::Block>(vals));
			// stack: block
		}
	};
	template<> struct action<grammar::wait_stmt> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: token expr
			auto expr = util::pop<ast::ValExpr>(s);
			s.pop_back();

			s.emplace_back(std::make_unique<ast::Wait>(std::move(expr)));
			// stack: wait
		}
	};
	template<> struct action<grammar::fnseq> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: expr, array?, anon_type?, tuple
			auto args = util::pop<ast::Tuple>(s);
			auto type = util::pop<ast::TypeExt>(s);
			auto inst = util::pop<ast::Array>(s);
			auto caller = util::pop<ast::Ast>(s);

			s.emplace_back(std::make_unique<ast::FnCall>(std::move(caller), std::move(type), std::move(args), std::move(inst)));
			// stack: fncall
		}
	};
	template<> struct action<grammar::fneps> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: expr | binding
			auto part = util::pop<ast::QualBinding>(s);
			if (part) s.emplace_back(std::make_unique<ast::Variable>(std::move(part)));
			// TODO: Consider pushing a Variale node instead (can add the function consideration in later)
			// stack: fncall | expr
		}
	};


	// Decorators
	template<> struct action<grammar::type> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: qualname array? ptr
			auto tkn = util::pop<ast::Token>(s);
			auto inst = util::pop<ast::Array>(s);
			auto name = util::pop<ast::QualBinding>(s);

			s.emplace_back(std::make_unique<ast::InstType>(std::move(name), std::move(inst), std::get<ast::PtrStyling>(tkn->val)));
		}
	};
	template<> struct action<grammar::unary> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			if (in.string() == "&") s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::DEREF));
			else if (in.string() == "!") s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::NOT));
			else if (in.string() == "-") s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::NA));
			else s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::NA));
		}
	};
	TOKEN(anot_glob, ast::UnaryType::NOT);
	template<> struct action<grammar::annotation> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: name glob? tuple?
			auto tup = util::pop<ast::Tuple>(s);
			auto glob = util::pop<ast::Token>(s);
			auto name = util::pop<ast::BasicBinding>(s);
			s.emplace_back(std::make_unique<ast::Annotation>(std::move(name), std::move(tup), glob != nullptr));
			// stack: annotation
		}
	};
	template<> struct action<grammar::mod_dec> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: KWD var*
			std::deque<ptr<ast::BasicBinding>> mod;
			while (util::at_node<ast::BasicBinding>(s))
				mod.push_front(util::pop<ast::BasicBinding>(s));

			s.pop_back();
			s.emplace_back(std::make_unique<ast::ModDec>(std::make_unique<ast::QualBinding>(mod)));
			// stack: mod_dec
		}
	};
	template<> struct action<grammar::mut_type> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: mut? type
			auto type = util::pop<ast::Type>(s);

			auto tkn = util::pop<ast::Token>(s);
			if (tkn)
				type->is_mut = std::holds_alternative<ast::KeywordType>(tkn->val)
					&& std::get<ast::KeywordType>(tkn->val)._to_integral() == ast::KeywordType::MUT;

			s.push_back(std::move(type));
			// stack: type
		}
	};
	template<> struct action<grammar::type_tuple> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: sentinel type*
			std::deque<ptr<ast::Type>> types;
			while (util::at_node<ast::Type>(s))
				types.push_front(util::pop<ast::Type>(s));

			s.pop_back();
			s.emplace_back(std::make_unique<ast::TupleType>(types));
			// stack: tupletype
		}
	};
	template<> struct action<grammar::inf> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: tuple? type
			auto type = util::pop<ast::Type>(s);
			if (util::view_as<ast::TupleType>(s.back()))
				s.emplace_back(std::make_unique<ast::FuncType>(util::pop<ast::TupleType>(s), std::move(type)));

			else
				s.push_back(std::move(type));
			// stack: type
		}
	};
	template<> struct action<grammar::gen_variance> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			if (in.string() == "+") s.emplace_back(std::make_unique<ast::Token>(ast::VarianceType::COVARIANT));
			else if (in.string() == "-") s.emplace_back(std::make_unique<ast::Token>(ast::VarianceType::CONTRAVARIANT));
			else s.emplace_back(std::make_unique<ast::Token>(ast::VarianceType::NA));
		}
	};
	template<> struct action<grammar::gen_subrel> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			if (in.string() == "::") s.emplace_back(std::make_unique<ast::Token>(ast::RelationType::IMPLS));
			else if (in.string() == "!:") s.emplace_back(std::make_unique<ast::Token>(ast::RelationType::NOT_IMPLS));
			else if (in.string() == ">")  s.emplace_back(std::make_unique<ast::Token>(ast::RelationType::SUPERTYPE));
			else  s.emplace_back(std::make_unique<ast::Token>(ast::RelationType::SUBTYPE));
		}
	};
	template<> struct action<grammar::gen_type> {
		static void apply(const pegtl::action_input& in, Stack& s) {
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
		}
	};
	template<> struct action<grammar::gen_val> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: name type? expr?
			auto expr = util::pop<ast::ValExpr>(s);
			auto type = util::pop<ast::Type>(s);
			auto name = util::pop<ast::BasicBinding>(s);
			s.emplace_back(std::make_unique<ast::ValueGeneric>(std::move(name), std::move(type), std::move(expr)));
			// stack: generic
		}
	};
	template<> struct action<grammar::use_any> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::ImportPiece>());
		}
	};
	template<> struct action<grammar::use_one> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: var
			s.emplace_back(std::make_unique<ast::ImportName>(util::pop<ast::BasicBinding>(s)));
			// stack: ImportName
		}
	};
	// TODO: Is this grammar wrong?
	template<> struct action<grammar::use_many> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: sentinel var*
			std::deque<ptr<ast::ImportName>> imps;
			while (util::at_node<ast::BasicBinding>(s))
				imps.push_front(std::make_unique<ast::ImportName>(util::pop<ast::BasicBinding>(s)));
		
			s.pop_back();
			s.emplace_back(std::make_unique<ast::ImportGroup>(imps));
			// stack: ImportGroup
		}
	};
	template<> struct action<grammar::use_path> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: MOD imps*
			std::deque<ptr<ast::ImportPiece>> imps;
			while (util::at_node<ast::ImportPiece>(s))
				imps.push_front(util::pop<ast::ImportPiece>(s));

			s.emplace_back(std::make_unique<ast::ImportPart>(imps));
			// stack: imp_part
		}
	};
	template<> struct action<grammar::use_elem> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: (var var?) | (typ typ?)
			auto bind1 = util::pop<ast::BasicBinding>(s);
			auto bind2 = util::pop<ast::BasicBinding>(s);
			if (bind2)
				s.emplace_back(std::make_unique<ast::ImportName>(std::move(bind2), std::move(bind1)));
			else
				s.emplace_back(std::make_unique<ast::ImportName>(std::move(bind1)));
			// stack: ImportName
		}
	};


	// Assignment
	template<> struct action<grammar::var_tuple> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: sentinel AssignPattern*
			std::deque<ptr<ast::AssignPattern>> pat;
			while (util::at_node<ast::AssignPattern>(s))
				pat.push_front(util::pop<ast::AssignPattern>(s));

			s.pop_back();
			s.emplace_back(std::make_unique<ast::AssignTuple>(pat));
			// stack: AssignTuple
		}
	};
	template<> struct action<grammar::var_pattern> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: binding
			if (util::at_node<ast::BasicBinding>(s))
				s.emplace_back(std::make_unique<ast::AssignName>(util::pop<ast::BasicBinding>(s)));
			// stack: VarPat
		}
	};
	INHERIT(var_type, var_pattern);
	template<> struct action<grammar::tuple_pat> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: sentinel pattern*
			std::deque<ptr<ast::Pattern>> vals;
			while (util::at_node<ast::Pattern>(s))
				vals.push_front(util::pop<ast::Pattern>(s));

			s.pop_back();
			s.emplace_back(std::make_unique<ast::PTuple>(vals));
			// stack: PTuple
		}
	};
	template<> struct action<grammar::pat_tuple> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: mut? PTuple
			std::iter_swap(std::rbegin(s) + 1, std::rbegin(s));
			
			auto tkn = util::pop<ast::Token>(s);
			if (tkn)
				util::view_as<ast::Pattern>(s.back())->is_mut = std::holds_alternative<ast::KeywordType>(tkn->val)
					&& std::get<ast::KeywordType>(tkn->val)._to_integral() == ast::KeywordType::MUT;

			else
				std::iter_swap(std::rbegin(s) + 1, std::rbegin(s));
			// stack: PTuple
		}
	};
	template<> struct action<grammar::pat_adt> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			//stack: binding PTuple?
			auto args = util::pop<ast::PTuple>(s);
			auto name = util::pop<ast::BasicBinding>(s);
			s.emplace_back(std::make_unique<ast::PAdt>(std::move(name), std::move(args)));
			//stack: PAdt
		}
	};
	template<> struct action<grammar::pat_var> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: mut? var
			auto pat = std::make_unique<ast::PNamed>(std::move(util::pop<ast::BasicBinding>(s)));

			auto tkn = util::pop<ast::Token>(s);
			if (tkn) {
				pat->is_mut = std::holds_alternative<ast::KeywordType>(tkn->val)
					&& std::get<ast::KeywordType>(tkn->val)._to_integral() == ast::KeywordType::MUT;

				if (!pat->is_mut)
					s.push_back(std::move(tkn));
			}

			s.push_back(std::move(pat));
			// stack: pattern
		}
	};
	template<> struct action<grammar::pat_any> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::Pattern>());
		}
	};
	template<> struct action<grammar::var_assign> {
		static void apply(const pegtl::action_input& in, Stack& s) {
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
		}
	};
	template<> struct action<grammar::adt_con> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: binding TupleType?
			auto args = util::pop<ast::TupleType>(s);
			auto name = util::pop<ast::BasicBinding>(s);
			s.emplace_back(std::make_unique<ast::Adt>(std::move(name), std::move(args)));
			// stack: Adt
		}
	};
	template<> struct action<grammar::rhs_inf> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: binding
			s.emplace_back(util::pop<ast::BasicBinding>(s));
			// stack: type
		}
	};
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
	template<> struct action<grammar::assign> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: visibility interface
			auto assign = util::pop<ast::Interface>(s);
			auto tkn = util::pop<ast::Token>(s);

			if (tkn && std::holds_alternative<ast::VisibilityType>(tkn->val))
				assign->setVisibility(std::get<ast::VisibilityType>(tkn->val));
			else {
			}

			s.push_back(std::move(assign));
			// stack: interface
		}
	};


	// Control
	template<> struct action<grammar::if_core> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: token test body
			auto body = util::pop<ast::ValExpr>(s);
			auto test = util::pop<ast::ValExpr>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::Branch>(std::move(test), std::move(body)));
			// stack: branch
		}
	};
	template<> struct action<grammar::if_dot_core> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: body token test
			auto iter = std::rbegin(s);
			std::iter_swap(iter + 2, iter + 1);
			std::iter_swap(iter, iter + 1);
			action<grammar::if_core>::apply(in, s);
			// stack: branch
		}
	};
	template<> struct action<grammar::elsif_rule> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: branch token test body
			auto body = util::pop<ast::ValExpr>(s);
			auto test = util::pop<ast::ValExpr>(s);
			s.pop_back();
			util::view_as<ast::Branch>(s.back())->addIf(std::move(test), std::move(body));
			// stack: branch
		}
	};
	template<> struct action<grammar::else_rule> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: branch token body
			auto body = util::pop<ast::ValExpr>(s);
			s.pop_back();
			util::view_as<ast::Branch>(s.back())->addElse(std::move(body));
			// stack: branch
		}
	};
	template<> struct action<grammar::loop> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: token body
			auto part = util::pop<ast::ValExpr>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::Loop>(std::move(part)));
			// stack: loop
		}
	};
	template<> struct action<grammar::dot_loop> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: body token
			std::iter_swap(std::rbegin(s), std::rbegin(s) + 1);
			action<grammar::loop>::apply(in, s);
			// stack: loop
		}
	};
	template<> struct action<grammar::while_l> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: token test body
			auto body = util::pop<ast::ValExpr>(s);
			auto test = util::pop<ast::ValExpr>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::While>(std::move(test), std::move(body)));
			// stack: while
		}
	};
	template<> struct action<grammar::dot_while> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: body token test
			auto iter = std::rbegin(s);
			std::iter_swap(iter + 2, iter + 1);
			std::iter_swap(iter, iter + 1);
			action<grammar::while_l>::apply(in, s);
			// stack: while
		}
	};
	template<> struct action<grammar::for_l> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: token pattern generator body
			auto body = util::pop<ast::ValExpr>(s);
			auto gen = util::pop<ast::ValExpr>(s);
			auto pattern = util::pop<ast::Pattern>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::For>(std::move(pattern), std::move(gen), std::move(body)));
			// stack: for
		}
	};
	template<> struct action<grammar::dot_for> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: body token pattern generator
			auto iter = std::rbegin(s);
			std::iter_swap(iter + 3, iter + 2);
			std::iter_swap(iter + 1, iter + 2);
			std::iter_swap(iter + 1, iter);
			action<grammar::for_l>::apply(in, s);
			// stack: for
		}
	};
	template<> struct action<grammar::jumps> {
		static void apply(const pegtl::action_input& in, Stack& s) {
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
		}
	};
	template<> struct action<grammar::dot_jmp> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: expr token
			std::iter_swap(std::rbegin(s), std::rbegin(s) + 1);
			action<grammar::jumps>::apply(in, s);
			// stack: jump
		}
	};
	template<> struct action<grammar::case_stmt> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: (sentinel | case) pattern* expr
			auto cas = std::make_unique<ast::Case>(std::move(util::pop<ast::ValExpr>(s)));

			std::deque<ptr<ast::Pattern>> pattern;
			while (util::at_node<ast::Pattern>(s))
				pattern.push_front(util::pop<ast::Pattern>(s));
			
			cas->setPattern(pattern);
			s.push_back(std::move(cas));
			// stack: (sentinel | case) case
		}
	};
	template<> struct action<grammar::match_expr> {
		static void apply(const pegtl::action_input& in, Stack& s) {
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
		}
	};
	template<> struct action<grammar::dot_match> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: expr token sentinel case+
			std::deque<ptr<ast::Case>> cases;
			while (util::at_node<ast::Case>(s))
				cases.push_front(util::pop<ast::Case>(s));

			s.pop_back();

			// stack: expr token

			s.pop_back();
			s.emplace_back(std::make_unique<ast::Match>(util::pop<ast::ValExpr>(s), cases));
			// stack: match
		}
	};


	// Expressions
	template<> struct action<grammar::_index_> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: (expr|index) expr
			auto expr = util::pop<ast::ValExpr>(s);
			if (util::at_node<ast::Index>(s))
				util::view_as<ast::Index>(s.back())->add(std::move(expr));
			else {
				auto expr2 = util::pop<ast::ValExpr>(s);
				s.emplace_back(std::make_unique<ast::Index>(std::move(expr2), std::move(expr)));
			}
			// stack: index
		}
	};
	template<> struct action<grammar::in_eps> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: unary? expr inf?
			auto typ = util::pop<ast::Type>(s);
			util::view_as<ast::ValExpr>(s.back())->type = std::move(typ);
			// stack: expr
		}
	};
	template<> struct action<grammar::range> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: expr, expr
			auto stop = util::pop<ast::ValExpr>(s);
			auto start = util::pop<ast::ValExpr>(s);
			s.emplace_back(std::make_unique<ast::Range>(std::move(start), std::move(stop)));
			// stack: range
		}
	};
	template<> struct action<grammar::impl_expr> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: IMPL qual
			auto qual = util::pop<ast::QualBinding>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::ImplExpr>(std::move(qual)));
			// stack: impl
		}
	};
	template<> struct action<grammar::_binary_> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: expr op expr
			std::iter_swap(std::rbegin(s) + 2, std::rbegin(s) + 1);

			// push arg tuple onto the stack
			std::deque<node> args;
			args.push_front(util::pop<ast::Ast>(s));
			args.push_front(util::pop<ast::Ast>(s));
			s.emplace_back(std::make_unique<ast::Tuple>(args));

			// stack: op tuple
			action<grammar::fnseq>::apply(in, s);
		}
	};
	template<> struct action<grammar::valexpr> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: mut? expr
			std::iter_swap(std::rbegin(s) + 1, std::rbegin(s));

			auto tkn = util::pop<ast::Token>(s);
			if (tkn) {
				auto* expr = util::view_as<ast::ValExpr>(s.back());
				expr->is_mut = std::holds_alternative<ast::KeywordType>(tkn->val)
					&& std::get<ast::KeywordType>(tkn->val)._to_integral() == ast::KeywordType::MUT;

				if (!expr->is_mut) {
					s.push_back(std::move(tkn));
					std::iter_swap(std::rbegin(s) + 1, std::rbegin(s));
				}

			} else
				std::iter_swap(std::rbegin(s) + 1, std::rbegin(s));
			// stack: expr
		}
	};
}

#undef SENTINEL
#undef INHERIT