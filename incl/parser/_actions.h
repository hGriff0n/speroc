#pragma once

#include "_grammar.h"
#include "util/_CompilationState.h"
#include "util/parser.h"

#include <algorithm>

/*
 * Simplify rule definition
 */
#define RULE(gram) \
	template<> struct action<grammar::gram> { \
		template<class Input> \
		static void apply(const Input& in, Stack& s)
// static void apply(const Input& in, Stack& s, CompilationState& state)
#define END }
#define PUSH(Node, ...) s.emplace_back(std::make_unique<ast::Node>(__VA_ARGS__, in.position()))
#define POP(Node) util::pop<ast::Node>(s)
#define INHERIT(gram, base) template<> struct action<grammar::gram> : action<grammar::base> {}


// Common case sentinel and token nodes
#define SENTINEL(gram)		RULE(gram) { s.emplace_back(ast::Sentinel{}); } END
#define TOKEN(gram, val)	RULE(gram) { PUSH(Token, val); } END


namespace spero::parser::actions {
	using namespace spero::compiler;

	template<class Rule>
	struct action : tao::pegtl::nothing<Rule> {};


	//
	// Sentinel Nodes
	//
	SENTINEL(obrace);
	SENTINEL(obrack);
	SENTINEL(oparen);


	//
	// Keyword Tokens (may remove a couple as it becomes beneficial)
	//
	TOKEN(kmut, ast::KeywordType::MUT);
	TOKEN(kloop, ast::KeywordType::LOOP);
	TOKEN(kif, ast::KeywordType::IF);
	TOKEN(kelsif, ast::KeywordType::ELSIF);
	TOKEN(kelse, ast::KeywordType::ELSE);
	TOKEN(kin, ast::KeywordType::F_IN);
	TOKEN(kmod, ast::KeywordType::MOD);
	TOKEN(kuse, ast::KeywordType::USE);
	TOKEN(klet, ast::VisibilityType::PROTECTED);
	TOKEN(kdef, ast::VisibilityType::PUBLIC);
	TOKEN(kstatic, ast::VisibilityType::STATIC);
	TOKEN(kimpl, ast::KeywordType::IMPL);
	TOKEN(kwhile, ast::KeywordType::WHILE);
	TOKEN(kfor, ast::KeywordType::FOR);
	TOKEN(kmatch, ast::KeywordType::MATCH);
	TOKEN(kdo, ast::KeywordType::DO);
	TOKEN(kwait, ast::KeywordType::WAIT);
	TOKEN(kyield, ast::KeywordType::YIELD);
	TOKEN(kret, ast::KeywordType::RET);
	TOKEN(kcontinue, ast::KeywordType::CONT);
	TOKEN(kbreak, ast::KeywordType::BREAK);


	//
	// Literals (done)
	//
	RULE(bin_body) {
		PUSH(Byte, in.string(), 2);
	} END;
	RULE(hex_body) {
		PUSH(Byte, in.string(), 16);
	} END;
	RULE(decimal) {
		auto str = in.string();
		if (str.find('.') != std::string::npos) {
			PUSH(Float, str);
		} else {
			PUSH(Int, str);
		}
	} END;
	RULE(char_body) {
		PUSH(Char, in.string()[0]);
	} END;
	RULE(str_body) {
		PUSH(String, util::escape(in.string()));
	} END;
	RULE(kfalse) {
		PUSH(Bool, false);
	} END;
	RULE(ktrue) {
		PUSH(Bool, true);
	} END;
	RULE(plambda) {
		PUSH(Future, false);
	} END;


	//
	// Atoms (done)
	//
	RULE(scope) {
		// stack: {} stmt*
		auto vals = util::popSeq<ast::Statement>(s);
		s.pop_back();
		PUSH(Block, std::move(vals));
		// stack: block
	} END;
	RULE(tuple) {
		// stack: {} valexpr*
		auto vals = util::popSeq<ast::ValExpr>(s);
		s.pop_back();
		PUSH(Tuple, std::move(vals));
		// stack: tuple
	} END;
	RULE(_array) {
		// stack: {} valexpr*
		auto vals = util::popSeq<ast::ValExpr>(s);
		s.pop_back();
		PUSH(Array, std::move(vals));
		// stack: array
	} END;
	RULE(lambda) {
		// stack: tuple expr
		auto body = POP(ValExpr);
		PUSH(Function, POP(Tuple), std::move(body));
		// stack: fndef
	} END;
	RULE(dot_eps) {
		PUSH(Future, true);
	} END;


	//
	// Identifiers
	//
	RULE(typ) {
		PUSH(BasicBinding, in.string(), ast::BindingType::TYPE);
	} END;
	RULE(var) {
		PUSH(BasicBinding, in.string(), ast::BindingType::VARIABLE);
	} END;
	RULE(op) {
		PUSH(BasicBinding, in.string(), ast::BindingType::OPERATOR);
	} END;
	RULE(mod_path) {
		// stack: (BasicBinding | QualifiedBinding) BasicBinding
		auto part = POP(BasicBinding);

		if (util::at_node<ast::BasicBinding>(s)) {
			PUSH(QualifiedBinding, POP(BasicBinding));
		}

		util::view_as<ast::QualifiedBinding>(s.back())->elems.push_back(std::move(part));
		// stack: QualifiedBinding
	} END;
	INHERIT(_mod_path, mod_path);
	RULE(qualtyp) {
		// stack: (QualifiedBinding | BasicBinding)? BasicBinding
		auto typ = POP(BasicBinding);

		if (util::at_node<ast::BasicBinding>(s)) {
			PUSH(QualifiedBinding, POP(BasicBinding));
		}

		if (util::at_node<ast::QualifiedBinding>(s)) {
			util::view_as<ast::QualifiedBinding>(s.back())->elems.push_back(std::move(typ));

		} else {
			PUSH(QualifiedBinding, std::move(typ));
		}
		// stack: QualifiedBinding
	} END;
	INHERIT(typname, qualtyp);
	RULE(pat_tuple) {
		// stack: {} pattern*
		auto pats = util::popSeq<ast::Pattern>(s);
		s.pop_back();
		PUSH(TuplePattern, std::move(pats));
		// stack: pattern
	} END;
	RULE(pat_name) {
		// stack: BasicBind cap BasicBind (due to pat_adt partially matching varname)
		auto var = POP(BasicBinding);

		// TODO: Find a way to just swap the cap and secondary bind elements (probably with dot_control)
		auto cap = POP(Token);
		s.pop_back();
		s.emplace_back(std::move(cap));

		PUSH(VarPattern, std::make_unique<ast::QualifiedBinding>(std::move(var), in.position()));
		// stack: pattern
	} END;
	RULE(pat_adt) {
		// stack: QualBind pattern?
		auto tup = POP(TuplePattern);
		PUSH(AdtPattern, POP(QualifiedBinding), std::move(tup));
		// stack: pattern
	} END;
	RULE(capture_desc) {
		// stack: kmut?
		bool is_ref = (in.string()[0] == '&');

		if (util::at_node<ast::Token>(s)) {
			if (util::view_as<ast::Token>(s.back())->get<ast::KeywordType>() == +ast::KeywordType::MUT) {
				s.pop_back();

				if (is_ref) {
					PUSH(Token, ast::CaptureType::MUTREF);
				} else {
					PUSH(Token, ast::CaptureType::MUT);
				}
			}

		} else if (is_ref) {
			PUSH(Token, ast::CaptureType::REF);
		}
		// stack: cap
	} END;
	RULE(capture) {
		// stack: cap? pattern
		auto pat = POP(Pattern);
		auto cap = POP(Token);

		if (cap) {
			pat->cap = cap->get<ast::CaptureType>();
		}
		s.emplace_back(std::move(pat));
		// stack: pattern
	} END;
	RULE(pat_any) {
		s.emplace_back(std::make_unique<ast::Pattern>(in.position()));
	} END;
	RULE(pat_lit) {
		PUSH(ValPattern, POP(ValExpr));
	} END;
	// assign_tuple
	// assign_pat


	//
	// Types
	//
	TOKEN(typ_view, ast::PtrStyling::VIEW);
	TOKEN(typ_ref, ast::PtrStyling::REF);
	TOKEN(typ_ptr, ast::PtrStyling::PTR);
	RULE(single_type) {
		// stack: QualBind array?
		auto gen = POP(Array);
		auto typname = POP(QualifiedBinding);

		if (gen) {
			PUSH(GenericType, std::move(typname), std::move(gen));
		} else {
			PUSH(SourceType, std::move(typname));
		}
		// stack: type
	} END;
	RULE(ref_type) {
		// stack: type ptrStyle?
		if (util::at_node<ast::Token>(s)) {
			auto tkn = POP(Token);

			if (tkn->holds<ast::PtrStyling>()) {
				util::view_as<ast::SourceType>(s.back())->_ptr = tkn->get<ast::PtrStyling>();
			}
		}
		// stack: type
	} END;
	RULE(tuple_type) {
		// stack: {} type*
		auto types = util::popSeq<ast::Type>(s);
		s.pop_back();
		PUSH(TupleType, std::move(types));
		// stack: tupleType
	} END;
	RULE(fn_type) {
		// stack: TupleType type
		auto ret = POP(Type);
		PUSH(FunctionType, POP(TupleType), std::move(ret));
		// stack: FunctionType
	} END;
	RULE(mut_type) {
		// stack: kmut? type
		auto typ = POP(Type);

		if (util::at_node<ast::Token>(s)) {
			auto tkn = POP(Token);

			if (tkn->holds<ast::KeywordType>()) {
				typ->is_mut = (tkn->get<ast::KeywordType>() == +ast::KeywordType::MUT);
			} else {
				s.push_back(std::move(tkn));
			}
		}

		s.push_back(std::move(typ));
		// stack: type
	} END;
	RULE(and_cont) {
		// stack: (type | and_type) type
		auto typ = POP(Type);

		if (!util::at_node<ast::AndType>(s)) {
			PUSH(AndType, POP(Type));
		}

		util::view_as<ast::AndType>(s.back())->elems.push_back(std::move(typ));
		// stack: and_type
	} END;
	RULE(or_cont) {
		// stack: (type | or_type) type
		auto typ = POP(Type);

		if (!util::at_node<ast::OrType>(s)) {
			PUSH(OrType, POP(Type));
		}

		util::view_as<ast::OrType>(s.back())->elems.push_back(std::move(typ));
		// stack: or_type
	} END;


	//
	// Decorators
	//
	RULE(annotation) {
		// stack: bind tuple?
		auto tup = POP(Tuple);
		PUSH(LocalAnnotation, POP(BasicBinding), std::move(tup));
		// stack: anot
	} END;
	RULE(ganot) {
		// stack: bind tuple?
		auto tup = POP(Tuple);
		PUSH(Annotation, POP(BasicBinding), std::move(tup));
		// stack: anot
	} END;
	// type_inf
	// variance
	// variadic
	// relation (?)
	// type_gen
	// val_gen
	// _generic
	RULE(adt) {
		// stack: typ type?
		 auto typ_args = POP(TupleType);
		PUSH(Adt, POP(BasicBinding), std::move(typ_args));
		// stack: adt
	} END;
	/*RULE(adt_dec) {
		auto adts = util::popSeq<ast::Adt>(s);
	} END;*/
	// arg
	// arg_tuple
	// anon_type


	//
	// Control
	//
	// forl
	// whilel
	// if_core
	// elsif_case
	// else_case
	// _case
	// matchs
	// jump
	// loop
	// dotloop
	// dotwhile
	// dotfor
	// dotbranch
	// dotmatch


	//
	// Expressions
	//
	// in_assign
	// actcall
	// type_const
	// named
	// valcall
	// fncall (X)
	// index_cont
	// unexpr


	//
	// Statements
	//
	// mod_dec
	// impl
	// mul_imp
	// alias
	// imp_alias
	// mod_alias
	// type_assign
	// asn_val
	// _interface
	// var_assign
	// assign


	// Binexpr Precedence
	INHERIT(binary_op1, op);
	INHERIT(binary_op2, op);
	INHERIT(binary_op3, op);
	INHERIT(binary_op4, op);
	INHERIT(binary_op5, op);
	INHERIT(binary_op6, op);
	INHERIT(binary_op7, op);
	// binary_cont1
	// binary_cont2
	// binary_cont3
	// binary_cont4
	// binary_cont5
	// binary_cont6
	// binary_cont7


	// Organizational Tagging
	// valexpr
	// statement

}

// Should be around 1300
#undef SENTINEL
#undef INHERIT
#undef TOKEN
#undef END
#undef RULE
#undef PUSH
#undef POP