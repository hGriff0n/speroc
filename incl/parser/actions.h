#pragma once

#include "grammar.h"
#include "ast.h"
#include "utils.h"

#include <algorithm>


// Specialized Definitions for Common Cases
#define SENTINEL(gram) \
template<> struct action<grammar::gram> {\
	template<class Input>\
	static void apply(const Input& in, Stack& s) { \
	s.emplace_back(ast::Sentinel{}); }}
#define TOKEN(gram, val) \
template<> struct action<grammar::gram> {\
	template<class Input>\
	static void apply(const Input& in, Stack& s) { \
	s.emplace_back(std::make_unique<ast::Token>(val, mkLoc(in))); }}
#define INHERIT(gram, base) \
template<> struct action<grammar::gram> : action<grammar::base> {}

namespace spero::parser::actions {
	using namespace spero::compiler;

	template<class Input>
	ast::Ast::Location mkLoc(const Input& in) {
		using Loc = compiler::ast::Ast::Location;

		Loc location{};
		location.byte = in.byte_in_line();
		location.line = in.line();
		location.src = in.source();

		return std::move(location);
	}

	template<class Rule>
	struct action : pegtl::nothing<Rule> {};

	// Sentinel Nodes
	SENTINEL(obrace);
	SENTINEL(obrack);
	SENTINEL(oparen);
	SENTINEL(range_op);


	// Literals
	template<> struct action<grammar::hex_body> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::Byte>(in.string(), 16, mkLoc(in)));
		}
	};
	template<> struct action<grammar::bin_body> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::Byte>(in.string(), 2, mkLoc(in)));
		}
	};
	template<> struct action<grammar::integer> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::Int>(in.string(), mkLoc(in)));
		}
	};
	template<> struct action<grammar::dec> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::Float>(in.string(), mkLoc(in)));
		}
	};
	template<> struct action<grammar::str_body> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::String>(util::escape(in.string()), mkLoc(in)));
		}
	};
	template<> struct action<grammar::char_body> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::Char>(util::escape(in.string())[0], mkLoc(in)));
		}
	};
	template<> struct action<grammar::b_false> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::Bool>(false, mkLoc(in)));
		}
	};
	template<> struct action<grammar::b_true> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::Bool>(true, mkLoc(in)));
		}
	};
	template<> struct action<grammar::tuple> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: sentinel vals*
			std::deque<ptr<ast::ValExpr>> vals;
			while (util::at_node<ast::ValExpr>(s))
				vals.push_front(util::pop<ast::ValExpr>(s));

			s.pop_back();
			s.emplace_back(std::make_unique<ast::Tuple>(std::move(vals), mkLoc(in)));
			// stack: tuple
		}
	};
	template<> struct action<grammar::array> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: sentinel vals*
			std::deque<ptr<ast::ValExpr>> vals;
			while (util::at_node<ast::ValExpr>(s))
				vals.push_front(util::pop<ast::ValExpr>(s));

			s.pop_back();
			s.emplace_back(std::make_unique<ast::Array>(std::move(vals), mkLoc(in)));
			// stack: array
		}
	};
	template<> struct action<grammar::fn_rettype> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: type expr
			auto fn = std::make_unique<ast::FnBody>(util::pop<ast::ValExpr>(s), false, mkLoc(in));
			fn->ret = std::move(util::pop<ast::Type>(s));
			s.push_back(std::move(fn));
			// stack: fnbody
		}
	};
	// Moved here from above fneps action to enable op_forward action
	template<> struct action<grammar::fnseq> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: expr array? anon_type? tuple
			auto args = util::pop<ast::Tuple>(s);
			auto type = util::pop<ast::TypeExt>(s);
			auto inst = util::pop<ast::Array>(s);

			// Handle the case where a function is called with a variable (prior actions leave the QualifiedBinding)
			if (util::at_node<ast::QualifiedBinding>(s))
				s.emplace_back(std::make_unique<ast::FnCall>(
					std::make_unique<ast::Variable>(util::pop<ast::QualifiedBinding>(s), mkLoc(in)),
					std::move(type), std::move(args), std::move(inst), mkLoc(in)));

			else
				s.emplace_back(std::make_unique<ast::FnCall>(
					util::pop<ast::ValExpr>(s), std::move(type), std::move(args), std::move(inst), mkLoc(in)));
			// stack: fncall
		}
	};
	template<> struct action<grammar::op_forward> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: bind expr

			if (!util::is_type<ast::Tuple>(s.back())) {
				std::deque<ptr<ast::ValExpr>> args;
				args.push_back(std::make_unique<ast::Future>(true, mkLoc(in)));
				args.push_back(std::move(util::pop<ast::ValExpr>(s)));

				s.emplace_back(std::make_unique<ast::Tuple>(std::move(args), mkLoc(in)));

			} else
				util::view_as<ast::Tuple>(s.back())->elems.push_front(std::make_unique<ast::Future>(true, mkLoc(in)));

			// stack: op tuple
			action<grammar::fnseq>::apply(in, s);
		}
	};
	template<> struct action<grammar::fn_forward> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: expr
			s.emplace_back(std::make_unique<ast::FnBody>(util::pop<ast::ValExpr>(s), true, mkLoc(in)));
			// stack: fnbody
		}
	};
	template<> struct action<grammar::fn_def> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: fnbody | expr
			if (!util::at_node<ast::FnBody>(s))
				s.emplace_back(std::make_unique<ast::FnBody>(util::pop<ast::ValExpr>(s), false, mkLoc(in)));
			// stack: fnbody
		}
	};
	template<> struct action<grammar::fn_or_tuple> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
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
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::BasicBinding>(in.string(), ast::BindingType::VARIABLE, mkLoc(in)));
		}
	};
	template<> struct action<grammar::typ> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::BasicBinding>(in.string(), ast::BindingType::TYPE, mkLoc(in)));
		}
	};
	template<> struct action<grammar::op> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// Produce a qualified binding because operators aren't included in name_path
			s.emplace_back(std::make_unique<ast::QualifiedBinding>(
				std::make_unique<ast::BasicBinding>(in.string(), ast::BindingType::OPERATOR, mkLoc(in)), mkLoc(in)
				));
		}
	};
	template<> struct action<grammar::name_path_part> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: qual? basic
			auto part = util::pop<ast::BasicBinding>(s);

			// Apparently this can be triggered in block ??
			if (part) {
				if (util::at_node<ast::QualifiedBinding>(s))
					util::view_as<ast::QualifiedBinding>(s.back())->elems.emplace_back(std::move(part));

				else
					s.emplace_back(std::make_unique<ast::QualifiedBinding>(std::move(part), mkLoc(in)));
			}
			// stack: qual
		}
	};
	SENTINEL(name_eps);
	template<> struct action<grammar::name_path> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: sentinel qual? basic
			auto part = util::pop<ast::BasicBinding>(s);

			// Apparently this can be triggered in block ??
			if (part) {
				if (util::at_node<ast::QualifiedBinding>(s))
					util::view_as<ast::QualifiedBinding>(s.back())->elems.emplace_back(std::move(part));

				else
					s.emplace_back(std::make_unique<ast::QualifiedBinding>(std::move(part), mkLoc(in)));
			}

			if (util::view_as<ast::Ast>(s.back()))
				std::iter_swap(std::rbegin(s), std::rbegin(s) + 1);
			s.pop_back();
			// stack: qual
		}
	};
	template<> struct action<grammar::typ_pointer> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			if (in.string() == "&") s.emplace_back(std::make_unique<ast::Token>(ast::PtrStyling::REF, mkLoc(in)));
			else if (in.string() == "*") s.emplace_back(std::make_unique<ast::Token>(ast::PtrStyling::PTR, mkLoc(in)));
			else if (in.string() == "..") s.emplace_back(std::make_unique<ast::Token>(ast::PtrStyling::VIEW, mkLoc(in)));
			else s.emplace_back(std::make_unique<ast::Token>(ast::PtrStyling::NA, mkLoc(in)));
		}
	};


	// Atoms
	SENTINEL(anon_sep);
	/*template<> struct action<grammar::anon_sep> {
	template<class Input>
	static void apply(const Input& in, Stack& s) {
	s.emplace_back(std::make_unique<ast::AnonTypeSeperator>());
	}
	};*/
	template<> struct action<grammar::anon_type> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: sentinel tuple? scope
			auto scp = util::pop<ast::Block>(s);
			auto tup = util::pop<ast::Tuple>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::TypeExt>(std::move(tup), std::move(scp), mkLoc(in)));
			// stack: anon
		}
	};
	template<> struct action<grammar::scope> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: sentinel vals*
			std::deque<ptr<ast::Stmt>> vals;
			while (util::at_node<ast::Stmt>(s))
				vals.push_front(util::pop<ast::Stmt>(s));

			s.pop_back();
			s.emplace_back(std::make_unique<ast::Block>(std::move(vals), mkLoc(in)));
			// stack: block
		}
	};
	template<> struct action<grammar::wait_stmt> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: token expr
			auto expr = util::pop<ast::ValExpr>(s);
			s.pop_back();

			s.emplace_back(std::make_unique<ast::Wait>(std::move(expr), mkLoc(in)));
			// stack: wait
		}
	};
	template<> struct action<grammar::fneps> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: expr | binding sentinel?
			bool sentinel_on_top = !util::view_as<ast::Ast>(s.back());
			if (sentinel_on_top) s.pop_back();

			auto name = util::pop<ast::QualifiedBinding>(s);
			if (name)
				s.emplace_back(std::make_unique<ast::Variable>(std::move(name), mkLoc(in)));


			if (sentinel_on_top) s.emplace_back(ast::Sentinel{});
			// stack: fncall | expr
		}
	};


	// Decorators
	template<> struct action<grammar::type> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: qualname array? ptr
			auto tkn = util::pop<ast::Token>(s);
			auto inst = util::pop<ast::Array>(s);
			auto name = util::pop<ast::QualifiedBinding>(s);

			s.emplace_back(std::make_unique<ast::GenericType>(std::move(name), std::move(inst), std::get<ast::PtrStyling>(tkn->value), mkLoc(in)));
		}
		//
	};
	template<> struct action<grammar::unary> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			auto un_c = in.string()[0];
			if (un_c == '&') s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::DEREF, mkLoc(in)));
			else if (un_c == '!') s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::NOT, mkLoc(in)));
			else if (un_c == '-') s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::MINUS, mkLoc(in)));
			else if (un_c == '~') s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::RESERVED, mkLoc(in)));
			else s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::NA, mkLoc(in)));
		}
	};
	template<> struct action<grammar::annotation> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: name glob? tuple?
			auto tup = util::pop<ast::Tuple>(s);
			auto name = util::pop<ast::BasicBinding>(s);

			s.emplace_back(std::make_unique<ast::LocalAnnotation>(std::move(name), std::move(tup), mkLoc(in)));
			// stack: annotation
		}
	};
	template<> struct action<grammar::global_annotation> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: name glob? tuple?
			auto tup = util::pop<ast::Tuple>(s);
			auto name = util::pop<ast::BasicBinding>(s);

			s.emplace_back(std::make_unique<ast::Annotation>(std::move(name), std::move(tup), mkLoc(in)));
			// stack: annotation
		}
	};
	template<> struct action<grammar::mod_dec> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: KWD var*
			std::deque<ptr<ast::BasicBinding>> mod;
			while (util::at_node<ast::BasicBinding>(s))
				mod.push_front(util::pop<ast::BasicBinding>(s));

			s.pop_back();
			s.emplace_back(std::make_unique<ast::ModDec>(std::make_unique<ast::QualifiedBinding>(std::move(mod), mkLoc(in)), mkLoc(in)));
			// stack: mod_dec
		}
	};
	template<> struct action<grammar::mut_type> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
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
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: sentinel type*
			std::deque<ptr<ast::Type>> types;
			while (util::at_node<ast::Type>(s))
				types.push_front(util::pop<ast::Type>(s));

			s.pop_back();
			s.emplace_back(std::make_unique<ast::TupleType>(std::move(types), mkLoc(in)));
			// stack: tupletype
		}
	};
	template<> struct action<grammar::inf_fn_type> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: tuple type
			auto ret = util::pop<ast::Type>(s);
			auto args = util::pop<ast::TupleType>(s);

			if (args)
				s.emplace_back(std::make_unique<ast::FuncType>(std::move(args), std::move(ret), mkLoc(in)));
			else
				s.emplace_back(std::move(ret));
			// stack: fntype
		}
	};
	template<> struct action<grammar::gen_variance> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			if (in.string() == "+") s.emplace_back(std::make_unique<ast::Token>(ast::VarianceType::COVARIANT, mkLoc(in)));
			else if (in.string() == "-") s.emplace_back(std::make_unique<ast::Token>(ast::VarianceType::CONTRAVARIANT, mkLoc(in)));
			else if (in.string() == "..") s.emplace_back(std::make_unique<ast::Token>(ast::VarianceType::VARIADIC, mkLoc(in)));
			else s.emplace_back(std::make_unique<ast::Token>(ast::VarianceType::INVARIANT, mkLoc(in)));
		}
	};
	template<> struct action<grammar::gen_subrel> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			if (in.string() == "::") s.emplace_back(std::make_unique<ast::Token>(ast::RelationType::IMPLS, mkLoc(in)));
			else if (in.string() == "!:") s.emplace_back(std::make_unique<ast::Token>(ast::RelationType::NOT_IMPLS, mkLoc(in)));
			else if (in.string() == ">")  s.emplace_back(std::make_unique<ast::Token>(ast::RelationType::SUPERTYPE, mkLoc(in)));
			else  s.emplace_back(std::make_unique<ast::Token>(ast::RelationType::SUBTYPE, mkLoc(in)));
		}
	};
	template<> struct action<grammar::gen_type> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: typ variance? (relation type)?
			auto type = util::pop<ast::Type>(s);
			auto rel = (type ? util::pop<ast::Token>(s) : nullptr);
			auto var = util::pop<ast::Token>(s);
			auto name = util::pop<ast::BasicBinding>(s);

			auto rel_val = rel ? std::get<ast::RelationType>(rel->value) : ast::RelationType::NA;
			auto var_val = var ? std::get<ast::VarianceType>(var->value) : ast::VarianceType::INVARIANT;

			s.emplace_back(std::make_unique<ast::TypeGeneric>(
				std::move(name), std::move(type),
				rel_val, var_val, mkLoc(in)
				));
			// stack: generic
		}
	};
	template<> struct action<grammar::gen_val> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: name type? expr?
			auto expr = util::pop<ast::ValExpr>(s);
			auto type = util::pop<ast::Type>(s);
			auto name = util::pop<ast::BasicBinding>(s);
			s.emplace_back(std::make_unique<ast::ValueGeneric>(std::move(name), std::move(type), std::move(expr), mkLoc(in)));
			// stack: generic
		}
	};
	template<> struct action<grammar::use_one> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: var
			s.emplace_back(std::make_unique<ast::ImportName>(util::pop<ast::BasicBinding>(s), mkLoc(in)));
			// stack: ImportName
		}
	};
	INHERIT(use_typ, use_one);
	template<> struct action<grammar::use_any> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::ImportPiece>(mkLoc(in)));
		}
	};
	template<> struct action<grammar::use_many> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: sentinel ImportName*
			std::deque<ptr<ast::ImportPiece>> imps;
			while (util::at_node<ast::ImportPiece>(s))
				imps.push_front(util::pop<ast::ImportPiece>(s));
			s.pop_back();
			s.emplace_back(std::make_unique<ast::ImportGroup>(std::move(imps), mkLoc(in)));
			// stack: ImportGroup
		}
	};
	template<> struct action<grammar::use_rebind> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: (var array? var?) | (typ array? typ?)
			auto name = util::pop<ast::BasicBinding>(s);
			auto inst = util::pop<ast::Array>(s);

			// TODO: Fix possibility for nullptr error
			auto* imp = util::view_as<ast::ImportName>(s.back());
			imp->generic_inst.swap(inst);
			imp->new_name.swap(name);
			// stack: ImportName
		}
	};
	INHERIT(alt_rebind, use_rebind);


	// Assignment
	template<> struct action<grammar::var_tuple> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: sentinel AssignPattern*
			std::deque<ptr<ast::AssignPattern>> pat;
			while (util::at_node<ast::AssignPattern>(s))
				pat.push_front(util::pop<ast::AssignPattern>(s));

			s.pop_back();
			s.emplace_back(std::make_unique<ast::AssignTuple>(std::move(pat), mkLoc(in)));
			// stack: AssignTuple
		}
	};
	template<> struct action<grammar::var_type> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: binding
			if (util::at_node<ast::BasicBinding>(s))
				s.emplace_back(std::make_unique<ast::AssignName>(util::pop<ast::BasicBinding>(s), mkLoc(in)));
			// stack: VarPat
		}
	};
	template<> struct action<grammar::var_pattern> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: binding
			if (util::at_node<ast::BasicBinding>(s))
				s.emplace_back(std::make_unique<ast::AssignName>(util::pop<ast::BasicBinding>(s), mkLoc(in)));

			if (util::at_node<ast::QualifiedBinding>(s) && util::view_as<ast::QualifiedBinding>(s.back())->elems.back()->type == +ast::BindingType::OPERATOR) {
				auto op = util::pop<ast::QualifiedBinding>(s);
				s.emplace_back(std::make_unique<ast::AssignName>(std::move(op->elems.back()), mkLoc(in)));
			}
			// stack: VarPat
		}
	};
	template<> struct action<grammar::capture_dec> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack:

			auto& str = in.string();
			if (str.size() == 0) {
				s.emplace_back(std::make_unique<ast::Token>(ast::CaptureType::NORM, mkLoc(in)));
				return;
			}

			// Is the current capture specifier a mut or a ref
			bool is_mut = str[0] == 'm';
			//bool is_ref = str[str.size() - 1] == '&';
			bool is_ref = (in.string().back() == '&');
			//bool is_ref = false;

			if (!is_ref)
				s.emplace_back(std::make_unique<ast::Token>(ast::CaptureType::MUT, mkLoc(in)));

			else if (!is_mut)
				s.emplace_back(std::make_unique<ast::Token>(ast::CaptureType::REF, mkLoc(in)));

			else
				s.emplace_back(std::make_unique<ast::Token>(ast::CaptureType::MUTREF, mkLoc(in)));
			// stack: cap
		}
	};
	template<> struct action<grammar::tuple_pat> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: sentinel pattern*
			std::deque<ptr<ast::Pattern>> vals;
			while (util::at_node<ast::Pattern>(s))
				vals.push_front(util::pop<ast::Pattern>(s));

			s.pop_back();
			s.emplace_back(std::make_unique<ast::PTuple>(std::move(vals), mkLoc(in)));
			// stack: PTuple
		}
	};
	template<> struct action<grammar::pat_tuple> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: mut? PTuple
			// stack: cap? PTuple
			std::iter_swap(std::rbegin(s) + 1, std::rbegin(s));

			auto tkn = util::pop<ast::Token>(s);
			if (tkn) {
				if (std::holds_alternative<ast::CaptureType>(tkn->value))
					util::view_as<ast::Pattern>(s.back())->cap = std::get<ast::CaptureType>(tkn->value);
				//if (std::holds_alternative<ast::KeywordType>(tkn->value)
				//&& std::get<ast::KeywordType>(tkn->value) == +ast::KeywordType::MUT)
				//util::view_as<ast::Pattern>(s.back())->cap = ast::CaptureType::MUT;

				else {
					s.push_back(std::move(tkn));
					std::iter_swap(std::rbegin(s) + 1, std::rbegin(s));
				}

			} else
				std::iter_swap(std::rbegin(s) + 1, std::rbegin(s));
			// stack: PTuple
		}
	};
	template<> struct action<grammar::pat_adt> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			//stack: binding PTuple?
			auto args = util::pop<ast::PTuple>(s);
			auto name = util::pop<ast::BasicBinding>(s);
			s.emplace_back(std::make_unique<ast::PAdt>(std::move(name), std::move(args), mkLoc(in)));
			//stack: PAdt
		}
	};
	template<> struct action<grammar::pat_var> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: mut? var
			auto pat = std::make_unique<ast::PNamed>(util::pop<ast::BasicBinding>(s), mkLoc(in));

			auto tkn = util::pop<ast::Token>(s);
			if (tkn) {
				if (std::holds_alternative<ast::CaptureType>(tkn->value))
					pat->cap = std::get<ast::CaptureType>(tkn->value);
				//if (std::holds_alternative<ast::KeywordType>(tkn->value)
				//&& std::get<ast::KeywordType>(tkn->value) == +ast::KeywordType::MUT)
				//pat->cap = ast::CaptureType::MUT;

				else
					s.push_back(std::move(tkn));
			}

			s.push_back(std::move(pat));
			// stack: pattern
		}
	};
	template<> struct action<grammar::pat_any> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::Pattern>(mkLoc(in)));
		}
	};
	template<> struct action<grammar::pat_val> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			s.emplace_back(std::make_unique<ast::PVal>(util::pop<ast::ValExpr>(s), mkLoc(in)));
		}
	};
	template<> struct action<grammar::in_binding> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: expr
			s.emplace_back(std::make_unique<ast::InAssign>(util::pop<ast::ValExpr>(s), mkLoc(in)));
			// stack: InAssignment
		}
	};
	template<> struct action<grammar::var_assign> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			using namespace std;

			// stack: pattern generic* (inf (expr | in)?) | (expr in?)
			auto inb = util::pop<ast::InAssign>(s);

			// Assemble the entire assignment statement
			auto val = util::pop<ast::ValExpr>(s);
			auto inf = util::pop<ast::Type>(s);

			// Get the generics list
			ast::GenArray gen;
			while (util::at_node<ast::GenericPart>(s))
				gen.push_front(util::pop<ast::GenericPart>(s));
			if (gen.size()) s.pop_back();

			auto bind = util::pop<ast::AssignPattern>(s);

			// Resolve the InAssignment possibility and push everything on the stack
			if (inb) {
				inb->binding = make_unique<ast::VarAssign>(move(bind), move(gen), move(val), move(inf), mkLoc(in));
				s.emplace_back(move(inb));

			} else if (val)
				s.emplace_back(make_unique<ast::VarAssign>(move(bind), move(gen), move(val), move(inf), mkLoc(in)));

			else
				s.emplace_back(make_unique<ast::Interface>(move(bind), move(gen), move(inf), mkLoc(in)));
			// stack: VarAssign
		}
	};
	template<> struct action<grammar::adt_con> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: binding TupleType?
			auto args = util::pop<ast::TupleType>(s);
			auto name = util::pop<ast::BasicBinding>(s);
			s.emplace_back(std::make_unique<ast::Adt>(std::move(name), std::move(args), mkLoc(in)));
			// stack: Adt
		}
	};
	template<> struct action<grammar::named_arg> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: var type?
			auto type = util::pop<ast::Type>(s);
			auto var = util::pop<ast::BasicBinding>(s);

			s.emplace_back(std::make_unique<ast::Variable>(std::make_unique<ast::QualifiedBinding>(std::move(var), mkLoc(in)), mkLoc(in)));
			util::view_as<ast::ValExpr>(s.back())->type = std::move(type);
			// stack: variable
		}
	};
	template<> struct action<grammar::con_tuple> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: sentinel variable*
			std::deque<ptr<ast::ValExpr>> vals;
			while (util::at_node<ast::Variable>(s))
				vals.push_front(util::pop<ast::ValExpr>(s));

			s.pop_back();
			s.emplace_back(std::make_unique<ast::Tuple>(std::move(vals), mkLoc(in)));
			// stack: tuple
		}
	};
	template<> struct action<grammar::rhs_inf> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: binding
			s.emplace_back(std::make_unique<ast::SourceType>(util::pop<ast::BasicBinding>(s), ast::PtrStyling::NA, mkLoc(in)));
			// stack: type
		}
	};
	template<> struct action<grammar::type_assign> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
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

			auto bind = util::pop<ast::AssignName>(s);
			s.emplace_back(std::make_unique<ast::TypeAssign>(std::move(bind), std::move(cons), std::move(gen), std::move(body), std::move(inherit), mkLoc(in)));
			// stack: TypeAssign
		}
	};
	template<> struct action<grammar::assign> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: vis (interface | inassign)
			if (util::at_node<ast::Interface>(s)) {
				auto assign = util::pop<ast::Interface>(s);
				auto tkn = util::pop<ast::Token>(s);

				if (tkn && std::holds_alternative<ast::VisibilityType>(tkn->value))
					assign->vis = std::get<ast::VisibilityType>(tkn->value);

				s.push_back(std::move(assign));

			} else {
				auto assign = util::pop<ast::InAssign>(s);
				auto tkn = util::pop<ast::Token>(s);

				if (tkn && std::holds_alternative<ast::VisibilityType>(tkn->value))
					assign->binding->vis = std::get<ast::VisibilityType>(tkn->value);

				s.push_back(std::move(assign));
			}
			// stack: interface | inassign
		}
	};


	// Control
	template<> struct action<grammar::if_core> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: token test body
			auto body = util::pop<ast::ValExpr>(s);
			auto test = util::pop<ast::ValExpr>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::IfElse>(std::move(test), std::move(body), mkLoc(in)));
			// stack: branch
		}
	};
	template<> struct action<grammar::if_dot_core> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: body token test
			auto iter = std::rbegin(s);
			std::iter_swap(iter + 2, iter + 1);
			std::iter_swap(iter + 1, iter);
			action<grammar::if_core>::apply(in, s);
			// stack: branch
		}
	};
	template<> struct action<grammar::elsif_rule> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: branch token test body
			auto body = util::pop<ast::ValExpr>(s);
			auto test = util::pop<ast::ValExpr>(s);
			s.pop_back();
			util::view_as<ast::IfElse>(s.back())->addBranch(std::move(test), std::move(body));
			// stack: branch
		}
	};
	template<> struct action<grammar::else_rule> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: branch token body
			auto body = util::pop<ast::ValExpr>(s);
			s.pop_back();
			util::view_as<ast::IfElse>(s.back())->else_branch = std::move(body);
			// stack: branch
		}
	};
	template<> struct action<grammar::fn_if_core> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: token test
			s.emplace_back(std::make_unique<ast::Future>(true, mkLoc(in)));
			action<grammar::if_core>::apply(in, s);
			// stack: branch
		}
	};
	template<> struct action<grammar::loop> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: token body
			auto part = util::pop<ast::ValExpr>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::Loop>(std::move(part), mkLoc(in)));
			// stack: loop
		}
	};
	template<> struct action<grammar::dot_loop> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: body token
			std::iter_swap(std::rbegin(s), std::rbegin(s) + 1);
			action<grammar::loop>::apply(in, s);
			// stack: loop
		}
	};
	template<> struct action<grammar::fn_dot_loop> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: token
			s.pop_back();
			s.emplace_back(std::make_unique<ast::Loop>(std::make_unique<ast::Future>(true, mkLoc(in)), mkLoc(in)));
			// stack: loop
		}
	};
	template<> struct action<grammar::while_l> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: token test body
			auto body = util::pop<ast::ValExpr>(s);
			auto test = util::pop<ast::ValExpr>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::While>(std::move(test), std::move(body), mkLoc(in)));
			// stack: while
		}
	};
	template<> struct action<grammar::dot_while> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: body token test
			auto iter = std::rbegin(s);
			std::iter_swap(iter + 2, iter + 1);
			std::iter_swap(iter, iter + 1);
			action<grammar::while_l>::apply(in, s);
			// stack: while
		}
	};
	template<> struct action<grammar::fn_dot_while> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: token test
			s.emplace_back(std::make_unique<ast::Future>(true, mkLoc(in)));
			action<grammar::while_l>::apply(in, s);
			// stack: while
		}
	};
	template<> struct action<grammar::for_l> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: token pattern generator body
			auto body = util::pop<ast::ValExpr>(s);
			auto gen = util::pop<ast::ValExpr>(s);
			auto pattern = util::pop<ast::Pattern>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::For>(std::move(pattern), std::move(gen), std::move(body), mkLoc(in)));
			// stack: for
		}
	};
	template<> struct action<grammar::dot_for> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: body token pattern generator
			auto iter = std::rbegin(s);
			std::iter_swap(iter + 3, iter + 2);			// stack: token body pattern generator
			std::iter_swap(iter + 2, iter + 1);			// stack: token pattern body generator
			std::iter_swap(iter + 1, iter);				// stack: token pattern generator body
			action<grammar::for_l>::apply(in, s);
			// stack: for
		}
	};
	template<> struct action<grammar::fn_dot_for> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: token pattern generator
			s.emplace_back(std::make_unique<ast::Future>(true, mkLoc(in)));
			action<grammar::for_l>::apply(in, s);
			// stack: for
		}
	};
	template<> struct action<grammar::jumps> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: token expr?
			auto expr = util::pop<ast::ValExpr>(s);
			auto tkn = std::get<ast::KeywordType>(util::pop<ast::Token>(s)->value);

			switch (tkn) {
				case ast::KeywordType::BREAK:
					s.emplace_back(std::make_unique<ast::Break>(std::move(expr), mkLoc(in)));
					break;
				case ast::KeywordType::CONT:
					s.emplace_back(std::make_unique<ast::Continue>(std::move(expr), mkLoc(in)));
					break;
				case ast::KeywordType::YIELD:
					s.emplace_back(std::make_unique<ast::YieldRet>(std::move(expr), mkLoc(in)));
					break;
				case ast::KeywordType::RET:
					s.emplace_back(std::make_unique<ast::Return>(std::move(expr), mkLoc(in)));
					break;
				default:
					break;
					//error
			}
			// stack: jump
		}
	};
	template<> struct action<grammar::dot_jmp> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: expr token
			std::iter_swap(std::rbegin(s), std::rbegin(s) + 1);
			action<grammar::jumps>::apply(in, s);
			// stack: jump
		}
	};
	template<> struct action<grammar::fn_dot_jmp> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: token
			s.emplace_back(std::make_unique<ast::Future>(true, mkLoc(in)));
			action<grammar::jumps>::apply(in, s);
			// stack: jump
		}
	};
	template<> struct action<grammar::case_stmt> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: (sentinel | case) pattern* (if expr)? expr
			auto expr = util::pop<ast::ValExpr>(s);
			auto if_guard = util::pop<ast::ValExpr>(s);
			if (if_guard) s.pop_back();

			// Collect all patterns
			std::deque<ptr<ast::Pattern>> pattern;
			while (util::at_node<ast::Pattern>(s))
				pattern.push_front(util::pop<ast::Pattern>(s));

			// And combine into a pattern tuple
			ptr<ast::PTuple> pt;
			if (pattern.size() == 1 && util::is_type<ast::PTuple>(pattern.back()))
				pt = util::dyn_cast<ast::PTuple>(std::move(pattern.back()));
			else
				pt = std::make_unique<ast::PTuple>(std::move(pattern), mkLoc(in));

			s.emplace_back(std::make_unique<ast::Case>(std::move(pt), std::move(expr), std::move(if_guard), mkLoc(in)));
			// stack: (sentinel | case) case
		}
	};
	template<> struct action<grammar::match_expr> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: token expr sentinel case+ cap
			s.pop_back();

			std::deque<ptr<ast::Case>> cases;
			while (util::at_node<ast::Case>(s))
				cases.push_front(util::pop<ast::Case>(s));

			s.pop_back();

			// stack: token expr

			auto match_val = util::pop<ast::ValExpr>(s);
			s.pop_back();

			s.emplace_back(std::make_unique<ast::Match>(std::move(match_val), std::move(cases), mkLoc(in)));
			// stack: match
		}
	};
	template<> struct action<grammar::dot_match> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: expr token sentinel case+
			std::deque<ptr<ast::Case>> cases;
			while (util::at_node<ast::Case>(s))
				cases.push_front(util::pop<ast::Case>(s));

			s.pop_back();

			// stack: expr token

			s.pop_back();
			s.emplace_back(std::make_unique<ast::Match>(util::pop<ast::ValExpr>(s), std::move(cases), mkLoc(in)));
			// stack: match
		}
	};
	template<> struct action<grammar::fn_dot_match> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: token sentinel case+
			std::deque<ptr<ast::Case>> cases;
			while (util::at_node<ast::Case>(s))
				cases.push_front(util::pop<ast::Case>(s));

			s.pop_back();

			// stack: expr token

			s.pop_back();
			auto expr = std::make_unique<ast::Future>(true, mkLoc(in));
			s.emplace_back(std::make_unique<ast::Match>(std::move(expr), std::move(cases), mkLoc(in)));
			// stack: match
		}
	};

	// Operator Precedence
	template<> struct action<grammar::op_prec_1> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// Produce a qualified binding because operators aren't included in name_path
			s.emplace_back(std::make_unique<ast::QualifiedBinding>(
				std::make_unique<ast::BasicBinding>(in.string(), ast::BindingType::OPERATOR, mkLoc(in)), mkLoc(in)
				));
		}
	};
	INHERIT(op_prec_2, op_prec_1);
	INHERIT(op_prec_3, op_prec_1);
	INHERIT(op_prec_4, op_prec_1);
	INHERIT(op_prec_5, op_prec_1);
	INHERIT(op_prec_6, op_prec_1);
	INHERIT(op_prec_7, op_prec_1);
	template<> struct action<grammar::_binary_prec_1> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: expr op expr -> op expr expr | expr op
			std::deque<ptr<ast::ValExpr>> args;
			bool not_curried = util::is_type<ast::ValExpr>(s.back());

			std::iter_swap(std::rbegin(s) + 1 + not_curried, std::rbegin(s) + not_curried);

			args.push_front(util::pop<ast::ValExpr>(s));
			if (not_curried) args.push_front(util::pop<ast::ValExpr>(s));

			s.emplace_back(std::make_unique<ast::Tuple>(std::move(args), mkLoc(in)));

			// stack: op tuple
			action<grammar::fnseq>::apply(in, s);
		}
	};
	INHERIT(_binary_prec_2, _binary_prec_1);
	INHERIT(_binary_prec_3, _binary_prec_1);
	INHERIT(_binary_prec_4, _binary_prec_1);
	INHERIT(_binary_prec_5, _binary_prec_1);
	INHERIT(_binary_prec_6, _binary_prec_1);
	INHERIT(_binary_prec_7, _binary_prec_1);

	// Expressions
	template<> struct action<grammar::_index_> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: (expr|index) expr
			auto expr = util::pop<ast::ValExpr>(s);
			if (util::at_node<ast::Index>(s))
				util::view_as<ast::Index>(s.back())->elems.push_back(std::move(expr));

			else {
				auto expr2 = util::pop<ast::ValExpr>(s);
				s.emplace_back(std::make_unique<ast::Index>(std::move(expr2), std::move(expr), mkLoc(in)));
			}
			// stack: index
		}
	};
	template<> struct action<grammar::un_eps> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: unary* expr
			auto expr = util::pop<ast::ValExpr>(s);

			while (util::at_node<ast::Token>(s)) {
				auto tkn = util::pop<ast::Token>(s);

				if (std::holds_alternative<ast::UnaryType>(tkn->value)) {
					expr = std::make_unique<ast::UnaryOpApp>(std::move(expr), std::get<ast::UnaryType>(tkn->value), mkLoc(in));

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
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: expr inf?
			auto typ = util::pop<ast::Type>(s);
			//if (typ) s.pop_back();		// NOTE: Removes sentinel from anon_sep production (from time where anon_sep="::")
			util::view_as<ast::ValExpr>(s.back())->type = std::move(typ);
			// stack: expr
		}
	};
	template<> struct action<grammar::_range_> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: expr, expr
			auto stop = util::pop<ast::ValExpr>(s);
			s.pop_back();
			auto start = util::pop<ast::ValExpr>(s);
			s.emplace_back(std::make_unique<ast::Range>(std::move(start), std::move(stop), mkLoc(in)));
			// stack: range
		}
	};
	template<> struct action<grammar::impl_expr> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: IMPL qual array? block?
			auto block = util::pop<ast::Block>(s);
			auto gen = util::pop<ast::Array>(s);
			auto qual = util::pop<ast::QualifiedBinding>(s);
			s.pop_back();

			s.emplace_back(std::make_unique<ast::ImplExpr>(
				std::make_unique<ast::GenericType>(std::move(qual), std::move(gen), ast::PtrStyling::NA, mkLoc(in)),
				std::move(block), mkLoc(in)));
			// stack: impl
		}
	};
	template<> struct action<grammar::mod_use> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: USE imp*
			std::deque<ptr<ast::ImportPiece>> imps;
			while (util::at_node<ast::ImportPiece>(s))
				imps.push_front(util::pop<ast::ImportPiece>(s));
			s.pop_back();
			s.emplace_back(std::make_unique<ast::ModImport>(std::move(imps), mkLoc(in)));
			// stack: Import
		}
	};
	template<> struct action<grammar::valexpr> {
		template<class Input>
		static void apply(const Input& in, Stack& s) {
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
		template<class Input>
		static void apply(const Input& in, Stack& s) {
			// stack: annot* stmt
			auto stmt = util::pop<ast::Stmt>(s);
			if (!stmt) return;

			while (util::at_node<ast::LocalAnnotation>(s))
				stmt->annots.push_front(util::pop<ast::LocalAnnotation>(s));

			s.push_back(std::move(stmt));
			// stack: stmt
		}
	};
}

#undef SENTINEL
#undef INHERIT