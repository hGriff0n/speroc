#pragma once

#include "grammar.h"
#include "ast/all.h"
#include "utils.h"

#include <algorithm>


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
			std::deque<ptr<ast::ValExpr>> vals;
			while (util::at_node<ast::ValExpr>(s))
				vals.push_front(util::pop<ast::ValExpr>(s));

			s.pop_back();
			s.emplace_back(std::make_unique<ast::Tuple>(std::move(vals)));
			// stack: tuple
		}
	};
	template<> struct action<grammar::array> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: sentinel vals*
			std::deque<ptr<ast::ValExpr>> vals;
			while (util::at_node<ast::ValExpr>(s))
				vals.push_front(util::pop<ast::ValExpr>(s));

			s.pop_back();
			s.emplace_back(std::make_unique<ast::Array>(std::move(vals)));
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
			// Produce a qualified binding because operators aren't included in name_path
			s.emplace_back(std::make_unique<ast::QualifiedBinding>(
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
				if (util::at_node<ast::QualifiedBinding>(s))
					util::view_as<ast::QualifiedBinding>(s.back())->parts.emplace_back(std::move(part));

				else
					s.emplace_back(std::make_unique<ast::QualifiedBinding>(std::move(part)));
			}
			// stack: qual
		}
	};
	SENTINEL(name_eps);
	template<> struct action<grammar::name_path> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: sentinel qual? basic
			auto part = util::pop<ast::BasicBinding>(s);

			// Apparently this can be triggered in block ??
			if (part) {
				if (util::at_node<ast::QualifiedBinding>(s))
					util::view_as<ast::QualifiedBinding>(s.back())->parts.emplace_back(std::move(part));

				else
					s.emplace_back(std::make_unique<ast::QualifiedBinding>(std::move(part)));
			}

			if (util::view_as<ast::Ast>(s.back()))
				std::iter_swap(std::rbegin(s), std::rbegin(s) + 1);
			s.pop_back();
			// stack: qual
		}
	};
	template<> struct action<grammar::typ_pointer> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			if (in.string() == "&") s.emplace_back(std::make_unique<ast::Token>(ast::PtrStyling::POINTER));
			else if (in.string() == "*") s.emplace_back(std::make_unique<ast::Token>(ast::PtrStyling::VIEW));
			else s.emplace_back(std::make_unique<ast::Token>(ast::PtrStyling::NA));
		}
	};


	// Atoms
	SENTINEL(anon_sep);
	template<> struct action<grammar::anon_type> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: sentinel tuple? scope
			auto scp = util::pop<ast::Block>(s);
			auto tup = util::pop<ast::Tuple>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::TypeExtension>(std::move(tup), std::move(scp)));
			// stack: anon
		}
	};
	template<> struct action<grammar::scope> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: sentinel vals*
			std::deque<ptr<ast::Ast>> vals;
			while (util::at_node<ast::Ast>(s))
				vals.push_front(util::pop<ast::Ast>(s));

			s.pop_back();
			s.emplace_back(std::make_unique<ast::Block>(std::move(vals)));
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
			auto type = util::pop<ast::TypeExtension>(s);
			auto inst = util::pop<ast::Array>(s);
			auto caller = util::pop<ast::Ast>(s);

			s.emplace_back(std::make_unique<ast::FnCall>(std::move(caller), std::move(type), std::move(args), std::move(inst)));
			// stack: fncall
		}
	};
	template<> struct action<grammar::fneps> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: expr | binding sentinel?
			bool sentinel_on_top = !util::view_as<ast::Ast>(s.back());
			if (sentinel_on_top) s.pop_back();

			auto name = util::pop<ast::QualifiedBinding>(s);
			if (name)
				s.emplace_back(std::make_unique<ast::Variable>(std::move(name)));


			if (sentinel_on_top) s.emplace_back(ast::Sentinel{});
			// stack: fncall | expr
		}
	};


	// Decorators
	template<> struct action<grammar::type> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: qualname array? ptr
			auto tkn = util::pop<ast::Token>(s);
			auto inst = util::pop<ast::Array>(s);
			auto name = util::pop<ast::QualifiedBinding>(s);

			s.emplace_back(std::make_unique<ast::GenericType>(std::move(name), std::move(inst), std::get<ast::PtrStyling>(tkn->value)));
		}
	};
	template<> struct action<grammar::unary> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			auto un_c = in.string()[0];
			if (un_c == '&') s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::DEREF));
			else if (un_c == '!') s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::NOT));
			else if (un_c == '-') s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::MINUS));
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
			s.emplace_back(std::make_unique<ast::ModDec>(std::make_unique<ast::QualifiedBinding>(std::move(mod))));
			// stack: mod_dec
		}
	};
	template<> struct action<grammar::mut_type> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: mut? type
			auto type = util::pop<ast::Type>(s);

			auto tkn = util::pop<ast::Token>(s);
			if (tkn) {
				type->is_mut = std::holds_alternative<ast::KeywordType>(tkn->value)
					&& std::get<ast::KeywordType>(tkn->value) == +ast::KeywordType::MUT;

				if (!type->is_mut)
					s.push_back(std::move(tkn));
			}

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
			s.emplace_back(std::make_unique<ast::TupleType>(std::move(types)));
			// stack: tupletype
		}
	};
	template<> struct action<grammar::inf_fn_type> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: tuple type
			auto ret = util::pop<ast::Type>(s);
			auto args = util::pop<ast::TupleType>(s);

			if (args)
				s.emplace_back(std::make_unique<ast::FunctionType>(std::move(args), std::move(ret)));
			else
				s.emplace_back(std::move(ret));
			// stack: fntype
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
			// stack: typ variance? (relation type)?
			auto type = util::pop<ast::Type>(s);
			auto rel = (type ? util::pop<ast::Token>(s) : nullptr);
			auto var = util::pop<ast::Token>(s);
			auto name = util::pop<ast::BasicBinding>(s);

			auto rel_val = rel ? std::get<ast::RelationType>(rel->value) : ast::RelationType::NA;
			auto var_val = var ? std::get<ast::VarianceType>(var->value) : ast::VarianceType::NA;

			s.emplace_back(std::make_unique<ast::TypeGeneric>(
				std::move(name), std::move(type),
				rel_val, var_val
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
	template<> struct action<grammar::use_one> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: var
			s.emplace_back(std::make_unique<ast::ImportName>(util::pop<ast::BasicBinding>(s)));
			// stack: ImportName
		}
	};
	template<> struct action<grammar::use_any> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::ImportPiece>());
		}
	};
	template<> struct action<grammar::use_many> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: sentinel ImportName*
			std::deque<ptr<ast::ImportPiece>> imps;
			while (util::at_node<ast::ImportPiece>(s))
				imps.push_front(util::pop<ast::ImportPiece>(s));
			s.pop_back();
			s.emplace_back(std::make_unique<ast::ImportGroup>(std::move(imps)));
			// stack: ImportGroup
		}
	};
	template<> struct action<grammar::use_rebind> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: (var var?) | (typ typ?)
			auto name = util::pop<ast::BasicBinding>(s);
			auto old = util::pop<ast::BasicBinding>(s);

			s.emplace_back(std::make_unique<ast::ImportName>(std::move(name), std::move(old)));
			// staack: ImportName
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
			s.emplace_back(std::make_unique<ast::AssignTuple>(std::move(pat)));
			// stack: AssignTuple
		}
	};
	template<> struct action<grammar::var_type> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: binding
			if (util::at_node<ast::BasicBinding>(s))
				s.emplace_back(std::make_unique<ast::AssignName>(util::pop<ast::BasicBinding>(s)));
			// stack: VarPat
		}
	};
	template<> struct action<grammar::var_pattern> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: binding
			if (util::at_node<ast::BasicBinding>(s))
				s.emplace_back(std::make_unique<ast::AssignName>(util::pop<ast::BasicBinding>(s)));

			if (util::at_node<ast::QualifiedBinding>(s) && util::view_as<ast::QualifiedBinding>(s.back())->parts.back()->type == +ast::BindingType::OPERATOR) {
				auto op = util::pop<ast::QualifiedBinding>(s);
				s.emplace_back(std::make_unique<ast::AssignName>(std::move(op->parts.back())));
			}
			// stack: VarPat
		}
	};
	template<> struct action<grammar::tuple_pat> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: sentinel pattern*
			std::deque<ptr<ast::Pattern>> vals;
			while (util::at_node<ast::Pattern>(s))
				vals.push_front(util::pop<ast::Pattern>(s));

			s.pop_back();
			s.emplace_back(std::make_unique<ast::PTuple>(std::move(vals)));
			// stack: PTuple
		}
	};
	template<> struct action<grammar::pat_tuple> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: mut? PTuple
			std::iter_swap(std::rbegin(s) + 1, std::rbegin(s));
			
			auto tkn = util::pop<ast::Token>(s);
			if (tkn) {
				auto* pat = util::view_as<ast::Pattern>(s.back());
				pat->is_mut = std::holds_alternative<ast::KeywordType>(tkn->value)
					&& std::get<ast::KeywordType>(tkn->value) == +ast::KeywordType::MUT;

				if (!pat->is_mut) {
					s.push_back(std::move(tkn));
					std::iter_swap(std::rbegin(s) + 1, std::rbegin(s));
				}
			} else
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
				pat->is_mut = std::holds_alternative<ast::KeywordType>(tkn->value)
					&& std::get<ast::KeywordType>(tkn->value) == +ast::KeywordType::MUT;

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
	template<> struct action<grammar::pat_val> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::PVal>(util::pop<ast::ValExpr>(s)));
		}
	};
	template<> struct action<grammar::var_assign> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: pattern generic? (inf expr?)|(inf? expr)
			auto val = util::pop<ast::ValExpr>(s);
			auto inf = util::pop<ast::Type>(s);

			// Get the generics list
			ast::GenArray gen;
			while (util::at_node<ast::GenericPart>(s))
				gen.push_front(util::pop<ast::GenericPart>(s));
			if (gen.size()) s.pop_back();

			auto bind = util::pop<ast::AssignPattern>(s);

			if (val)
				s.emplace_back(std::make_unique<ast::VarAssign>(std::move(bind), std::move(gen), std::move(val), std::move(inf)));
			else
				s.emplace_back(std::make_unique<ast::Interface>(std::move(bind), std::move(gen), std::move(inf)));
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
			std::deque<ptr<ast::Ast>> cons;
			while (util::at_node<ast::Adt>(s) || util::at_node<ast::Tuple>(s))
				cons.push_front(util::pop<ast::Ast>(s));

			auto inherit = util::pop<ast::Type>(s);

			// Get the generics list
			ast::GenArray gen;
			while (util::at_node<ast::GenericPart>(s))
				gen.push_front(util::pop<ast::GenericPart>(s));
			if (gen.size()) s.pop_back();

			auto bind = util::pop<ast::AssignPattern>(s);
			s.emplace_back(std::make_unique<ast::TypeAssign>(std::move(bind), std::move(cons), std::move(gen), std::move(body), std::move(inherit)));
			// stack: TypeAssign
		}
	};
	template<> struct action<grammar::assign> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: visibility interface
			auto assign = util::pop<ast::Interface>(s);
			auto tkn = util::pop<ast::Token>(s);

			if (tkn && std::holds_alternative<ast::VisibilityType>(tkn->value))
				assign->vis = std::get<ast::VisibilityType>(tkn->value);
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
			util::view_as<ast::Branch>(s.back())->addBranch(std::move(test), std::move(body));
			// stack: branch
		}
	};
	template<> struct action<grammar::else_rule> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: branch token body
			auto body = util::pop<ast::ValExpr>(s);
			s.pop_back();
			util::view_as<ast::Branch>(s.back())->addBranch(std::move(body));
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
			auto tkn = std::get<ast::KeywordType>(util::pop<ast::Token>(s)->value);

			switch (tkn) {
				case ast::KeywordType::BREAK:
					s.emplace_back(std::make_unique<ast::Break>(std::move(expr)));
					break;
				case ast::KeywordType::CONT:
					s.emplace_back(std::make_unique<ast::Continue>(std::move(expr)));
					break;
				case ast::KeywordType::YIELD:
					s.emplace_back(std::make_unique<ast::YieldRet>(std::move(expr)));
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
			auto expr = util::pop<ast::ValExpr>(s);

			std::deque<ptr<ast::Pattern>> pattern;
			while (util::at_node<ast::Pattern>(s))
				pattern.push_front(util::pop<ast::Pattern>(s));
			
			ptr<ast::PTuple> pt;
			if (pattern.size() == 1 && util::is_type<ast::PTuple>(pattern.back()))
				pt = util::dyn_cast<ast::PTuple>(std::move(pattern.back()));
			else
				pt = std::make_unique<ast::PTuple>(std::move(pattern));
			
			s.emplace_back(std::make_unique<ast::Case>(std::move(pt), std::move(expr)));
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

			s.emplace_back(std::make_unique<ast::Match>(std::move(match_val), std::move(cases)));
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
			s.emplace_back(std::make_unique<ast::Match>(util::pop<ast::ValExpr>(s), std::move(cases)));
			// stack: match
		}
	};


	// Expressions
	template<> struct action<grammar::_index_> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: (expr|index) expr
			auto expr = util::pop<ast::ValExpr>(s);
			if (util::at_node<ast::Index>(s))
				util::view_as<ast::Index>(s.back())->elems.push_back(std::move(expr));
			else {
				auto expr2 = util::pop<ast::ValExpr>(s);
				s.emplace_back(std::make_unique<ast::Index>(std::move(expr2), std::move(expr)));
			}
			// stack: index
		}
	};
	template<> struct action<grammar::un_eps> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: unary* expr
			auto expr = util::pop<ast::ValExpr>(s);

			while (util::at_node<ast::Token>(s)) {
				auto tkn = util::pop<ast::Token>(s);
				
				if (std::holds_alternative<ast::UnaryType>(tkn->value)) {
					expr = std::make_unique<ast::UnaryApp>(std::move(expr), std::get<ast::UnaryType>(tkn->value));

				} else {
					s.emplace_back(std::move(tkn));
					break;
				}
			}

			s.emplace_back(std::move(expr));
			// stack: expr
		}
	};
	template<> struct action<grammar::in_eps> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: expr inf?
			auto typ = util::pop<ast::Type>(s);
			//if (typ) s.pop_back();		// NOTE: Removes sentinel from anon_sep production (from time where anon_sep="::")
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
			auto qual = util::pop<ast::QualifiedBinding>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::ImplExpr>(std::move(qual)));
			// stack: impl
		}
	};
	template<> struct action<grammar::mod_use> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: USE imp*
			std::deque<ptr<ast::ImportPiece>> imps;
			while (util::at_node<ast::ImportPiece>(s))
				imps.push_front(util::pop<ast::ImportPiece>(s));
			s.pop_back();
			s.emplace_back(std::make_unique<ast::ModImport>(std::move(imps)));
			// stack: Import
		}
	};
	template<> struct action<grammar::_binary_> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: expr op expr -> op expr expr
			std::iter_swap(std::rbegin(s) + 2, std::rbegin(s) + 1);

			// push arg tuple onto the stack
			std::deque<ptr<ast::ValExpr>> args;
			args.push_front(util::pop<ast::ValExpr>(s));
			args.push_front(util::pop<ast::ValExpr>(s));
			s.emplace_back(std::make_unique<ast::Tuple>(std::move(args)));

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
				expr->is_mut = std::holds_alternative<ast::KeywordType>(tkn->value)
					&& std::get<ast::KeywordType>(tkn->value) == +ast::KeywordType::MUT;

				if (!expr->is_mut) {
					s.push_back(std::move(tkn));
					std::iter_swap(std::rbegin(s) + 1, std::rbegin(s));
				}

			} else
				std::iter_swap(std::rbegin(s) + 1, std::rbegin(s));
			// stack: expr
		}
	};
	template<> struct action<grammar::stmt> {
		static void apply(const pegtl::action_input& in, Stack& s) {
			// stack: annot* stmt
			auto stmt = util::pop<ast::Stmt>(s);

			while (util::at_node<ast::Annotation>(s))
				stmt->annots.push_front(util::pop<ast::Annotation>(s));

			s.push_back(std::move(stmt));
			// stack: stmt
		}
	};
}

#undef SENTINEL
#undef INHERIT