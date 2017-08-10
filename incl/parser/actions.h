#pragma once

#include "grammar.h"
#include "ast.h"
#include "util/parser.h"

#include <algorithm>


// Specialized Definitions for Common Cases
#define SENTINEL(gram) \
template<> struct action<grammar::gram> {\
	template<class Input>\
	static void apply(const Input& in, Stack& s, CompilationState& state) { \
	s.emplace_back(ast::Sentinel{}); }}
#define TOKEN(gram, val) \
template<> struct action<grammar::gram> {\
	template<class Input>\
	static void apply(const Input& in, Stack& s, CompilationState& state) { \
	s.emplace_back(std::make_unique<ast::Token>(val, in.position())); }}
#define INHERIT(gram, base) \
template<> struct action<grammar::gram> : action<grammar::base> {}

namespace spero::parser::actions {
	using namespace spero::compiler;

	template<class Rule>
	struct action : tao::pegtl::nothing<Rule> {};

	// Sentinel Nodes
	SENTINEL(obrace);
	SENTINEL(obrack);
	SENTINEL(oparen);
	SENTINEL(range_op);


	// Literals
	template<> struct action<grammar::hex_body> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			s.emplace_back(std::make_unique<ast::Byte>(in.string(), 16, in.position()));
		}
	};
	template<> struct action<grammar::bin_body> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			s.emplace_back(std::make_unique<ast::Byte>(in.string(), 2, in.position()));
		}
	};
	template<> struct action<grammar::integer> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			s.emplace_back(std::make_unique<ast::Int>(in.string(), in.position()));
		}
	};
	template<> struct action<grammar::dec> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			s.emplace_back(std::make_unique<ast::Float>(in.string(), in.position()));
		}
	};
	template<> struct action<grammar::str_body> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			s.emplace_back(std::make_unique<ast::String>(util::escape(in.string()), in.position()));
		}
	};
	template<> struct action<grammar::char_body> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			s.emplace_back(std::make_unique<ast::Char>(util::escape(in.string())[0], in.position()));
		}
	};
	template<> struct action<grammar::b_false> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			s.emplace_back(std::make_unique<ast::Bool>(false, in.position()));
		}
	};
	template<> struct action<grammar::b_true> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			s.emplace_back(std::make_unique<ast::Bool>(true, in.position()));
		}
	};
	template<> struct action<grammar::lambda_placeholder> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			s.emplace_back(std::make_unique<ast::Future>(false, in.position()));
		}
	};
	template<> struct action<grammar::tuple> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: sentinel vals*
			auto vals = util::popSeq<ast::ValExpr>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::Tuple>(std::move(vals), in.position()));
			// stack: tuple
		}
	};
	template<> struct action<grammar::_array> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: sentinel vals*
			auto vals = util::popSeq<ast::ValExpr>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::Array>(std::move(vals), in.position()));
			// stack: array
		}
	};
	template<> struct action<grammar::fn_rettype> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: type expr
			auto fn = std::make_unique<ast::FnBody>(util::pop<ast::ValExpr>(s), false, in.position());
			fn->ret = std::move(util::pop<ast::Type>(s));
			s.push_back(std::move(fn));
			// stack: fnbody
		}
	};
	template<> struct action<grammar::fnseq> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: expr array? anon_type? tuple
			auto args = util::pop<ast::Tuple>(s);
			auto type = util::pop<ast::TypeExt>(s);
			auto inst = util::pop<ast::Array>(s);

			// Handle the case where a function is called with a variable (prior actions leave the QualifiedBinding)
			if (util::at_node<ast::QualifiedBinding>(s))
				s.emplace_back(std::make_unique<ast::FnCall>(
					std::make_unique<ast::Variable>(util::pop<ast::QualifiedBinding>(s), in.position()),
					std::move(type), std::move(args), std::move(inst), in.position()));

			else
				s.emplace_back(std::make_unique<ast::FnCall>(
					util::pop<ast::ValExpr>(s), std::move(type), std::move(args), std::move(inst), in.position()));
			// stack: fncall
		}
	};
	template<> struct action<grammar::op_forward> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: bind expr

			if (!util::is_type<ast::Tuple>(s.back())) {
				auto left = util::pop<ast::ValExpr>(s);

				std::deque<ptr<ast::ValExpr>> args;
				args.push_back(std::make_unique<ast::Future>(true, in.position()));
				args.push_back(left ? std::move(left) : std::make_unique<ast::Future>(true, in.position()));

				s.emplace_back(std::make_unique<ast::Tuple>(std::move(args), in.position()));

			} else
				util::view_as<ast::Tuple>(s.back())->elems.push_front(std::make_unique<ast::Future>(true, in.position()));

			// stack: op tuple
			action<grammar::fnseq>::apply(in, s, state);
		}
	};
	template<> struct action<grammar::fn_forward> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: expr
			s.emplace_back(std::make_unique<ast::FnBody>(util::pop<ast::ValExpr>(s), true, in.position()));
			// stack: fnbody
		}
	};
	template<> struct action<grammar::fn_def> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: fnbody | expr
			if (!util::at_node<ast::FnBody>(s)) {
				auto body = util::pop<ast::ValExpr>(s);
				while (!util::at_node<ast::Tuple>(s)) s.pop_back();
				s.emplace_back(std::make_unique<ast::FnBody>(std::move(body), false, in.position()));
			}
			// stack: fnbody
		}
	};
	template<> struct action<grammar::fn_or_tuple> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
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
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			s.emplace_back(std::make_unique<ast::BasicBinding>(in.string(), ast::BindingType::VARIABLE, in.position()));
		}
	};
	INHERIT(var_core, var);
	template<> struct action<grammar::typ> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			s.emplace_back(std::make_unique<ast::BasicBinding>(in.string(), ast::BindingType::TYPE, in.position()));
		}
	};
	template<> struct action<grammar::op> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// Produce a qualified binding because operators aren't included in name_path
			s.emplace_back(std::make_unique<ast::QualifiedBinding>(
				std::make_unique<ast::BasicBinding>(in.string(), ast::BindingType::OPERATOR, in.position()), in.position()
				));
		}
	};
	template<> struct action<grammar::name_path_part> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: qual? basic
			auto part = util::pop<ast::BasicBinding>(s);

			// Apparently this can be triggered in block ??
			if (part) {
				if (util::at_node<ast::QualifiedBinding>(s))
					util::view_as<ast::QualifiedBinding>(s.back())->elems.emplace_back(std::move(part));

				else
					s.emplace_back(std::make_unique<ast::QualifiedBinding>(std::move(part), in.position()));
			}
			// stack: qual
		}
	};
	SENTINEL(name_eps);
	template<> struct action<grammar::name_path> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: sentinel qual? basic
			auto part = util::pop<ast::BasicBinding>(s);

			// Apparently this can be triggered in block ??
			if (part) {
				if (util::at_node<ast::QualifiedBinding>(s))
					util::view_as<ast::QualifiedBinding>(s.back())->elems.emplace_back(std::move(part));

				else
					s.emplace_back(std::make_unique<ast::QualifiedBinding>(std::move(part), in.position()));
			}

			if (util::view_as<ast::Ast>(s.back()))
				std::iter_swap(std::rbegin(s), std::rbegin(s) + 1);
			s.pop_back();
			// stack: qual
		}
	};
	template<> struct action<grammar::typ_pointer> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			if (in.string() == "&") s.emplace_back(std::make_unique<ast::Token>(ast::PtrStyling::REF, in.position()));
			else if (in.string() == "*") s.emplace_back(std::make_unique<ast::Token>(ast::PtrStyling::PTR, in.position()));
			else if (in.string() == "..") s.emplace_back(std::make_unique<ast::Token>(ast::PtrStyling::VIEW, in.position()));
			else s.emplace_back(std::make_unique<ast::Token>(ast::PtrStyling::NA, in.position()));
		}
	};


	// Atoms
	SENTINEL(anon_sep);
	/*template<> struct action<grammar::anon_sep> {
	template<class Input>
	static void apply(const Input& in, Stack& s, CompilationState& state) {
	s.emplace_back(std::make_unique<ast::AnonTypeSeperator>());
	}
	};*/
	template<> struct action<grammar::anon_type> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: sentinel tuple? scope
			auto scp = util::pop<ast::Block>(s);
			auto tup = util::pop<ast::Tuple>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::TypeExt>(std::move(tup), std::move(scp), in.position()));
			// stack: anon
		}
	};
	template<> struct action<grammar::scope> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: sentinel vals*
			auto vals = util::popSeq<ast::Stmt>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::Block>(std::move(vals), in.position()));
			// stack: block
		}
	};
	template<> struct action<grammar::wait_stmt> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: token expr
			auto expr = util::pop<ast::ValExpr>(s);
			s.pop_back();

			s.emplace_back(std::make_unique<ast::Wait>(std::move(expr), in.position()));
			// stack: wait
		}
	};
	template<> struct action<grammar::fneps> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: expr | binding sentinel?
			bool sentinel_on_top = !util::view_as<ast::Ast>(s.back());
			if (sentinel_on_top) s.pop_back();

			auto name = util::pop<ast::QualifiedBinding>(s);
			if (name)
				s.emplace_back(std::make_unique<ast::Variable>(std::move(name), in.position()));

			if (sentinel_on_top) s.emplace_back(ast::Sentinel{});
			// stack: fncall | expr
		}
	};
	template<> struct action<grammar::unary_op> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: token
			s.pop_back();
			auto loc = in.position();
			s.emplace_back(
				std::make_unique<ast::Variable>(
					std::make_unique<ast::QualifiedBinding>(
						std::make_unique<ast::BasicBinding>(
							in.string(), ast::BindingType::OPERATOR, loc)
						, loc), loc));
			// stack: expr
		}
	};


	// Decorators
	template<> struct action<grammar::_type> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: qualname array?
			auto inst = util::pop<ast::Array>(s);
			auto name = util::pop<ast::QualifiedBinding>(s);

			s.emplace_back(std::make_unique<ast::GenericType>(std::move(name), std::move(inst), ast::PtrStyling::NA, in.position()));
			// stack: type
		}
	};
	template<> struct action<grammar::type> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: type ptr
			auto tkn = util::pop<ast::Token>(s);
			util::view_as<ast::GenericType>(s.back())->_ptr = std::get<ast::PtrStyling>(tkn->value);
			// stack: type
		}
	};
	template<> struct action<grammar::unary> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			auto un_c = in.string()[0];
			if (un_c == '&') s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::DEREF, in.position()));
			else if (un_c == '!') s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::NOT, in.position()));
			else if (un_c == '-') s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::MINUS, in.position()));
			else if (un_c == '~') s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::RESERVED, in.position()));
			else s.emplace_back(std::make_unique<ast::Token>(ast::UnaryType::NA, in.position()));
		}
	};
	template<> struct action<grammar::annotation> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: name glob? tuple?
			auto tup = util::pop<ast::Tuple>(s);
			auto name = util::pop<ast::BasicBinding>(s);

			s.emplace_back(std::make_unique<ast::LocalAnnotation>(std::move(name), std::move(tup), in.position()));
			// stack: annotation
		}
	};
	template<> struct action<grammar::global_annotation> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: name glob? tuple?
			auto tup = util::pop<ast::Tuple>(s);
			auto name = util::pop<ast::BasicBinding>(s);

			s.emplace_back(std::make_unique<ast::Annotation>(std::move(name), std::move(tup), in.position()));
			// stack: annotation
		}
	};
	template<> struct action<grammar::mod_dec> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: KWD var*
			auto mod = util::popSeq<ast::BasicBinding>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::ModDec>(std::make_unique<ast::QualifiedBinding>(std::move(mod), in.position()), in.position()));
			// stack: mod_dec
		}
	};
	template<> struct action<grammar::inf_tuple_type> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: sentinel type*
			auto types = util::popSeq<ast::Type>(s);

			s.pop_back();
			s.emplace_back(std::make_unique<ast::TupleType>(std::move(types), in.position()));
			// stack: tupletype
		}
	};
	template<> struct action<grammar::inf_fn_type> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: tuple type
			auto ret = util::pop<ast::Type>(s);
			s.emplace_back(std::make_unique<ast::FuncType>(util::pop<ast::TupleType>(s), std::move(ret), in.position()));
			// stack: fntype
		}
	};
	SENTINEL(inf_eps);
	template<> struct action<grammar::inf_mut_type> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: type
			if (in.string()[0] == 'm') util::view_as<ast::Type>(s.back())->is_mut = true;
			// stack: type
		}
	};
	template<> struct action<grammar::_inf_junc_and> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: sentinel type*
			auto types = util::popSeq<ast::Type>(s);
			s.emplace_back(std::make_unique<ast::AndType>(std::move(types), in.position()));
			// stack: sentinel andtype
		}
	};
	template<> struct action<grammar::_inf_junc_or> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: sentinel type*
			auto types = util::popSeq<ast::Type>(s);
			s.emplace_back(std::make_unique<ast::OrType>(std::move(types), in.position()));
			// stack: sentinel ortype
		}
	};
	template<> struct action<grammar::inf_type> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: sentinel type
			std::iter_swap(std::rbegin(s) + 1, std::rbegin(s));
			s.pop_back();
			// stack: type
		}
	};

	template<> struct action<grammar::gen_variance> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			if (in.string() == "+") s.emplace_back(std::make_unique<ast::Token>(ast::VarianceType::COVARIANT, in.position()));
			else if (in.string() == "-") s.emplace_back(std::make_unique<ast::Token>(ast::VarianceType::CONTRAVARIANT, in.position()));
			else s.emplace_back(std::make_unique<ast::Token>(ast::VarianceType::INVARIANT, in.position()));
		}
	};
	template<> struct action<grammar::gen_variadic> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			if (in.string() == "..") s.emplace_back(std::make_unique<ast::Token>(ast::VarianceType::VARIADIC, in.position()));
		}
	};
	template<> struct action<grammar::gen_subrel> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			if (in.string() == "::") s.emplace_back(std::make_unique<ast::Token>(ast::RelationType::IMPLS, in.position()));
			else if (in.string() == "!:") s.emplace_back(std::make_unique<ast::Token>(ast::RelationType::NOT_IMPLS, in.position()));
			else if (in.string() == ">")  s.emplace_back(std::make_unique<ast::Token>(ast::RelationType::SUPERTYPE, in.position()));
			else  s.emplace_back(std::make_unique<ast::Token>(ast::RelationType::SUBTYPE, in.position()));
		}
	};
	TOKEN(gen_subrel_eps, ast::RelationType::NA);
	template<> struct action<grammar::gen_type> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: typ variance? variadic? (relation type)?
			auto typs = util::popSeq<ast::Type>(s);
			auto rel_val = std::get<ast::RelationType>(util::pop<ast::Token>(s)->value);
			auto variadic = util::pop<ast::Token>(s);
			auto variance = util::pop<ast::Token>(s);
			auto name = util::pop<ast::BasicBinding>(s);

			// Extract the token values from `seq` and `var`, accounting for possible syntactic non-inclusion
			auto seq_val = variadic ? std::get<ast::VarianceType>(variadic->value) : ast::VarianceType::INVARIANT;
			auto var_val = variance ? std::get<ast::VarianceType>(variance->value) : ast::VarianceType::INVARIANT;

			s.emplace_back(std::make_unique<ast::TypeGeneric>(
				std::move(name), std::make_unique<ast::AndType>(std::move(typs), in.position()),
				rel_val, seq_val, var_val, in.position()
				));
			// stack: generic
		}
	};
	template<> struct action<grammar::gen_val> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: name type? expr?
			auto expr = util::pop<ast::ValExpr>(s);
			auto type = util::pop<ast::Type>(s);
			auto name = util::pop<ast::BasicBinding>(s);
			s.emplace_back(std::make_unique<ast::ValueGeneric>(std::move(name), std::move(type), std::move(expr), in.position()));
			// stack: generic
		}
	};
	template<> struct action<grammar::use_one> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: var
			s.emplace_back(std::make_unique<ast::ImportName>(util::pop<ast::BasicBinding>(s), in.position()));
			// stack: ImportName
		}
	};
	INHERIT(use_typ, use_one);
	template<> struct action<grammar::use_any> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			s.emplace_back(std::make_unique<ast::ImportPiece>(in.position()));
		}
	};
	template<> struct action<grammar::use_many> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: sentinel ImportName*
			auto imps = util::popSeq<ast::ImportPiece>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::ImportGroup>(std::move(imps), in.position()));
			// stack: ImportGroup
		}
	};
	template<> struct action<grammar::use_rebind> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
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
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: sentinel AssignPattern*
			auto pat = util::popSeq<ast::AssignPattern>(s);

			s.pop_back();
			s.emplace_back(std::make_unique<ast::AssignTuple>(std::move(pat), in.position()));
			// stack: AssignTuple
		}
	};
	template<> struct action<grammar::var_type> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: binding
			if (util::at_node<ast::BasicBinding>(s))
				s.emplace_back(std::make_unique<ast::AssignName>(util::pop<ast::BasicBinding>(s), in.position()));
			// stack: VarPat
		}
	};
	template<> struct action<grammar::var_pattern> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: binding
			if (util::at_node<ast::BasicBinding>(s))
				s.emplace_back(std::make_unique<ast::AssignName>(util::pop<ast::BasicBinding>(s), in.position()));

			if (util::at_node<ast::QualifiedBinding>(s) && util::view_as<ast::QualifiedBinding>(s.back())->elems.back()->type == +ast::BindingType::OPERATOR) {
				auto op = util::pop<ast::QualifiedBinding>(s);
				s.emplace_back(std::make_unique<ast::AssignName>(std::move(op->elems.back()), in.position()));
			}
			// stack: VarPat
		}
	};
	template<> struct action<grammar::capture_dec> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack:

			auto& str = in.string();
			if (str.size() == 0) {
				s.emplace_back(std::make_unique<ast::Token>(ast::CaptureType::NORM, in.position()));
				return;
			}

			// Is the current capture specifier a mut or a ref
			bool is_mut = str[0] == 'm';
			//bool is_ref = str[str.size() - 1] == '&';
			bool is_ref = (in.string().back() == '&');
			//bool is_ref = false;

			if (!is_ref)
				s.emplace_back(std::make_unique<ast::Token>(ast::CaptureType::MUT, in.position()));

			else if (!is_mut)
				s.emplace_back(std::make_unique<ast::Token>(ast::CaptureType::REF, in.position()));

			else
				s.emplace_back(std::make_unique<ast::Token>(ast::CaptureType::MUTREF, in.position()));
			// stack: cap
		}
	};
	template<> struct action<grammar::tuple_pat> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: sentinel pattern*
			auto vals = util::popSeq<ast::Pattern>(s);

			s.pop_back();
			s.emplace_back(std::make_unique<ast::PTuple>(std::move(vals), in.position()));
			// stack: PTuple
		}
	};
	template<> struct action<grammar::pat_tuple> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
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
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			//stack: binding PTuple?
			auto args = util::pop<ast::PTuple>(s);
			auto name = util::pop<ast::BasicBinding>(s);
			s.emplace_back(std::make_unique<ast::PAdt>(std::move(name), std::move(args), in.position()));
			//stack: PAdt
		}
	};
	template<> struct action<grammar::pat_var> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: mut? var type?
			auto type = util::pop<ast::Type>(s);
			auto pat = std::make_unique<ast::PNamed>(util::pop<ast::BasicBinding>(s), std::move(type), in.position());

			auto tkn = util::pop<ast::Token>(s);
			if (tkn) {
				if (std::holds_alternative<ast::CaptureType>(tkn->value))
					pat->cap = std::get<ast::CaptureType>(tkn->value);

				else
					s.push_back(std::move(tkn));
			}

			s.push_back(std::move(pat));
			// stack: pattern
		}
	};
	template<> struct action<grammar::pat_any> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			s.emplace_back(std::make_unique<ast::Pattern>(in.position()));
		}
	};
	template<> struct action<grammar::pat_val> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			s.emplace_back(std::make_unique<ast::PVal>(util::pop<ast::ValExpr>(s), in.position()));
		}
	};
	template<> struct action<grammar::in_binding> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: expr
			s.emplace_back(std::make_unique<ast::InAssign>(util::pop<ast::ValExpr>(s), in.position()));
			// stack: InAssignment
		}
	};
	template<> struct action<grammar::var_assign> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
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
				inb->binding = make_unique<ast::VarAssign>(move(bind), move(gen), move(val), move(inf), in.position());
				s.emplace_back(move(inb));

			} else if (val)
				s.emplace_back(make_unique<ast::VarAssign>(move(bind), move(gen), move(val), move(inf), in.position()));

			else
				s.emplace_back(make_unique<ast::Interface>(move(bind), move(gen), move(inf), in.position()));
			// stack: VarAssign
		}
	};
	template<> struct action<grammar::adt_con> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: binding TupleType?
			auto args = util::pop<ast::TupleType>(s);
			auto name = util::pop<ast::BasicBinding>(s);
			s.emplace_back(std::make_unique<ast::Adt>(std::move(name), std::move(args), in.position()));
			// stack: Adt
		}
	};
	template<> struct action<grammar::named_arg> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: var type?
			auto type = util::pop<ast::Type>(s);
			auto var = util::pop<ast::BasicBinding>(s);

			s.emplace_back(std::make_unique<ast::Variable>(std::make_unique<ast::QualifiedBinding>(std::move(var), in.position()), in.position()));
			util::view_as<ast::ValExpr>(s.back())->type = std::move(type);
			// stack: variable
		}
	};
	template<> struct action<grammar::con_tuple> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: sentinel variable*
			auto vals = util::popSeq<ast::ValExpr>(s);

			s.pop_back();
			s.emplace_back(std::make_unique<ast::Tuple>(std::move(vals), in.position()));
			// stack: tuple
		}
	};
	template<> struct action<grammar::rhs_inf> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: binding
			s.emplace_back(std::make_unique<ast::SourceType>(util::pop<ast::BasicBinding>(s), ast::PtrStyling::NA, in.position()));
			if (util::is_type<ast::Token>(*(std::rbegin(s) + 1)))
				std::iter_swap(std::rbegin(s), std::rbegin(s) + 1);
			// stack: type
		}
	};
	template<> struct action<grammar::type_assign> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: binding generics* inf? (adt|tuple)* scope
			auto body = util::pop<ast::Block>(s);

			// Get the constructor list (can't use `util::popSeq`)
			std::deque<ptr<ast::Ast>> cons;
			while (util::at_node<ast::Adt>(s) || util::at_node<ast::Tuple>(s))
				cons.push_front(util::pop<ast::Ast>(s));

			auto mut = util::pop<ast::Token>(s);
			auto inherit = util::pop<ast::Type>(s);

			// Get the generics list
			ast::GenArray gen;
			while (util::at_node<ast::GenericPart>(s))
				gen.push_front(util::pop<ast::GenericPart>(s));
			if (gen.size()) s.pop_back();

			auto bind = util::pop<ast::AssignName>(s);
			s.emplace_back(std::make_unique<ast::TypeAssign>(std::move(bind), std::move(cons), std::move(gen), std::move(body), std::move(inherit), (bool)mut, in.position()));
			// stack: TypeAssign
		}
	};
	template<> struct action<grammar::assign> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
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
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: token test body
			auto body = util::pop<ast::ValExpr>(s);
			auto test = util::pop<ast::ValExpr>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::IfElse>(std::move(test), std::move(body), in.position()));
			// stack: branch
		}
	};
	template<> struct action<grammar::if_dot_core> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: body token test
			auto iter = std::rbegin(s);
			std::iter_swap(iter + 2, iter + 1);
			std::iter_swap(iter + 1, iter);
			action<grammar::if_core>::apply(in, s, state);
			// stack: branch
		}
	};
	template<> struct action<grammar::elsif_rule> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
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
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: branch token body
			auto body = util::pop<ast::ValExpr>(s);
			s.pop_back();
			util::view_as<ast::IfElse>(s.back())->else_branch = std::move(body);
			// stack: branch
		}
	};
	template<> struct action<grammar::fn_if_core> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: token test
			s.emplace_back(std::make_unique<ast::Future>(true, in.position()));
			action<grammar::if_core>::apply(in, s, state);
			// stack: branch
		}
	};
	template<> struct action<grammar::loop> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: token body
			auto part = util::pop<ast::ValExpr>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::Loop>(std::move(part), in.position()));
			// stack: loop
		}
	};
	template<> struct action<grammar::dot_loop> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: body token
			std::iter_swap(std::rbegin(s), std::rbegin(s) + 1);
			action<grammar::loop>::apply(in, s, state);
			// stack: loop
		}
	};
	template<> struct action<grammar::fn_dot_loop> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: token
			s.pop_back();
			s.emplace_back(std::make_unique<ast::Loop>(std::make_unique<ast::Future>(true, in.position()), in.position()));
			// stack: loop
		}
	};
	template<> struct action<grammar::while_l> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: token test body
			auto body = util::pop<ast::ValExpr>(s);
			auto test = util::pop<ast::ValExpr>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::While>(std::move(test), std::move(body), in.position()));
			// stack: while
		}
	};
	template<> struct action<grammar::dot_while> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: body token test
			auto iter = std::rbegin(s);
			std::iter_swap(iter + 2, iter + 1);
			std::iter_swap(iter, iter + 1);
			action<grammar::while_l>::apply(in, s, state);
			// stack: while
		}
	};
	template<> struct action<grammar::fn_dot_while> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: token test
			s.emplace_back(std::make_unique<ast::Future>(true, in.position()));
			action<grammar::while_l>::apply(in, s, state);
			// stack: while
		}
	};
	template<> struct action<grammar::for_l> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: token pattern generator body
			auto body = util::pop<ast::ValExpr>(s);
			auto gen = util::pop<ast::ValExpr>(s);
			auto pattern = util::pop<ast::Pattern>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::For>(std::move(pattern), std::move(gen), std::move(body), in.position()));
			// stack: for
		}
	};
	template<> struct action<grammar::dot_for> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: body token pattern generator
			auto iter = std::rbegin(s);
			std::iter_swap(iter + 3, iter + 2);			// stack: token body pattern generator
			std::iter_swap(iter + 2, iter + 1);			// stack: token pattern body generator
			std::iter_swap(iter + 1, iter);				// stack: token pattern generator body
			action<grammar::for_l>::apply(in, s, state);
			// stack: for
		}
	};
	template<> struct action<grammar::fn_dot_for> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: token pattern generator
			s.emplace_back(std::make_unique<ast::Future>(true, in.position()));
			action<grammar::for_l>::apply(in, s, state);
			// stack: for
		}
	};
	template<> struct action<grammar::jumps> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: token expr?
			auto expr = util::pop<ast::ValExpr>(s);
			auto tkn = std::get<ast::KeywordType>(util::pop<ast::Token>(s)->value);

			switch (tkn) {
				case ast::KeywordType::BREAK:
					s.emplace_back(std::make_unique<ast::Break>(std::move(expr), in.position()));
					break;
				case ast::KeywordType::CONT:
					s.emplace_back(std::make_unique<ast::Continue>(std::move(expr), in.position()));
					break;
				case ast::KeywordType::YIELD:
					s.emplace_back(std::make_unique<ast::YieldRet>(std::move(expr), in.position()));
					break;
				case ast::KeywordType::RET:
					s.emplace_back(std::make_unique<ast::Return>(std::move(expr), in.position()));
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
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: expr token
			std::iter_swap(std::rbegin(s), std::rbegin(s) + 1);
			action<grammar::jumps>::apply(in, s, state);
			// stack: jump
		}
	};
	template<> struct action<grammar::fn_dot_jmp> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: token
			s.emplace_back(std::make_unique<ast::Future>(true, in.position()));
			action<grammar::jumps>::apply(in, s, state);
			// stack: jump
		}
	};
	template<> struct action<grammar::case_stmt> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: (sentinel | case) pattern* (if expr)? expr
			auto expr = util::pop<ast::ValExpr>(s);
			auto if_guard = util::pop<ast::ValExpr>(s);
			if (if_guard) s.pop_back();

			// Collect all patterns
			auto pattern = util::popSeq<ast::Pattern>(s);

			// And combine into a pattern tuple
			ptr<ast::PTuple> pt;
			if (pattern.size() == 1 && util::is_type<ast::PTuple>(pattern.back()))
				pt = util::dyn_cast<ast::PTuple>(std::move(pattern.back()));
			else
				pt = std::make_unique<ast::PTuple>(std::move(pattern), in.position());

			s.emplace_back(std::make_unique<ast::Case>(std::move(pt), std::move(expr), std::move(if_guard), in.position()));
			// stack: (sentinel | case) case
		}
	};
	template<> struct action<grammar::match_expr> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: token expr sentinel case+ cap
			s.pop_back();
			auto cases = util::popSeq<ast::Case>(s);
			s.pop_back();

			// stack: token expr

			auto match_val = util::pop<ast::ValExpr>(s);
			s.pop_back();

			s.emplace_back(std::make_unique<ast::Match>(std::move(match_val), std::move(cases), in.position()));
			// stack: match
		}
	};
	template<> struct action<grammar::dot_match> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: expr token sentinel case+
			auto cases = util::popSeq<ast::Case>(s);
			s.pop_back();

			// stack: expr token

			s.pop_back();
			s.emplace_back(std::make_unique<ast::Match>(util::pop<ast::ValExpr>(s), std::move(cases), in.position()));
			// stack: match
		}
	};
	template<> struct action<grammar::fn_dot_match> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: token sentinel case+
			auto cases = util::popSeq<ast::Case>(s);
			s.pop_back();

			// stack: expr token

			s.pop_back();
			auto expr = std::make_unique<ast::Future>(true, in.position());
			s.emplace_back(std::make_unique<ast::Match>(std::move(expr), std::move(cases), in.position()));
			// stack: match
		}
	};

	// Operator Precedence
	template<> struct action<grammar::op_prec_1> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// Produce a qualified binding because operators aren't included in name_path
			s.emplace_back(std::make_unique<ast::QualifiedBinding>(
				std::make_unique<ast::BasicBinding>(in.string(), ast::BindingType::OPERATOR, in.position()), in.position()
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
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: expr op expr | expr op
			auto rhs = util::pop<ast::ValExpr>(s);
			auto op = util::pop<ast::QualifiedBinding>(s)->elems.back()->name;
			auto lhs = util::pop<ast::ValExpr>(s);

			// Handle optional left-side currying
			if (!rhs) rhs = std::make_unique<ast::Future>(false, in.position());

			// Handle reassignment/compound operators
			// TODO: Need to account for possibility for types to overload compound operators
			// TODO: Adjust when operators are changed to a method call (can move back to one pathing)
			if (op.back() == '=') {
				auto _lhs = util::dyn_cast<ast::Variable>(std::move(lhs));

				// TODO: Check that 'lhs' is a variable

				// Handle compound reassignments
				if (op.size() != 1) {
					_lhs = std::make_unique<ast::Variable>(_lhs->name->toString(), in.position());

					rhs = std::make_unique<ast::BinOpCall>(std::move(_lhs), std::move(rhs), op.substr(0, op.size() - 1), in.position());
				}

				s.emplace_back(std::make_unique<ast::Reassign>(std::move(_lhs), std::move(rhs), in.position()));

			// Or push the operator call
			} else {
				s.emplace_back(std::make_unique<ast::BinOpCall>(std::move(lhs), std::move(rhs), std::move(op), in.position()));
			}
			// stack: Reassign | BinOpCall
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
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: (expr|index) expr
			auto expr = util::pop<ast::ValExpr>(s);
			if (util::at_node<ast::Index>(s))
				util::view_as<ast::Index>(s.back())->elems.push_back(std::move(expr));

			else {
				auto expr2 = util::pop<ast::ValExpr>(s);
				s.emplace_back(std::make_unique<ast::Index>(std::move(expr2), std::move(expr), in.position()));
			}
			// stack: index
		}
	};
	template<> struct action<grammar::un_eps> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: unary* expr
			auto expr = util::pop<ast::ValExpr>(s);

			while (util::at_node<ast::Token>(s)) {
				auto tkn = util::pop<ast::Token>(s);

				if (std::holds_alternative<ast::UnaryType>(tkn->value)) {
					expr = std::make_unique<ast::UnaryOpApp>(std::move(expr), std::get<ast::UnaryType>(tkn->value), in.position());

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
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: expr inf?
			auto typ = util::pop<ast::Type>(s);
			//if (typ) s.pop_back();		// NOTE: Removes sentinel from anon_sep production (from time where anon_sep="::")
			util::view_as<ast::ValExpr>(s.back())->type = std::move(typ);
			// stack: expr
		}
	};
	template<> struct action<grammar::_range_> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: expr, expr
			auto stop = util::pop<ast::ValExpr>(s);
			s.pop_back();
			auto start = util::pop<ast::ValExpr>(s);
			s.emplace_back(std::make_unique<ast::Range>(std::move(start), std::move(stop), in.position()));
			// stack: range
		}
	};
	template<> struct action<grammar::impl_expr> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: IMPL qual array? block?
			auto block = util::pop<ast::Block>(s);
			auto gen = util::pop<ast::Array>(s);
			auto qual = util::pop<ast::QualifiedBinding>(s);
			s.pop_back();

			s.emplace_back(std::make_unique<ast::ImplExpr>(
				std::make_unique<ast::GenericType>(std::move(qual), std::move(gen), ast::PtrStyling::NA, in.position()),
				std::move(block), in.position()));
			// stack: impl
		}
	};
	template<> struct action<grammar::mod_use> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
			// stack: USE imp*
			auto imps = util::popSeq<ast::ImportPiece>(s);
			s.pop_back();
			s.emplace_back(std::make_unique<ast::ModImport>(std::move(imps), in.position()));
			// stack: Import
		}
	};
	template<> struct action<grammar::valexpr> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
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
	template<> struct action<grammar::anot_expr> {
		template<class Input>
		static void apply(const Input& in, Stack& s, CompilationState& state) {
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