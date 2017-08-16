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
#define INHERIT(gram, base) template<> struct action<grammar::gram> : action<grammar::base> {}


// Common case sentinel and token nodes
#define SENTINEL(gram)		RULE(gram) { s.emplace_back(ast::Sentinel{}); } END
#define TOKEN(gram, val)	RULE(gram) { PUSH(Token, val); } END


namespace spero::parser::actions {
	using namespace spero::compiler;

	template<class Rule>
	struct action : tao::pegtl::nothing<Rule> {};

	// Sentinel Nodes
	SENTINEL(obrace);
	SENTINEL(obrack);
	SENTINEL(oparen);


	// Keyword Tokens (may remove a couple as it becomes beneficial)
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


	// Literals
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
	// Plambda


	// Atoms
	RULE(scope) {

	} END;
	RULE(tuple) {

	} END;
	RULE(_array) {

	} END;
	// Lambda
	// DotFn


	// Identifiers


	// Types
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
		auto part = util::pop<ast::BasicBinding>(s);

		if (util::at_node<ast::BasicBinding>(s)) {
			PUSH(QualifiedBinding, util::pop<ast::BasicBinding>(s));
		}

		util::view_as<ast::QualifiedBinding>(s.back())->elems.push_back(std::move(part));
		// stack: QualifiedBinding
	} END;
	INHERIT(_mod_path, mod_path);
	RULE(qualtyp) {
		// stack: (QualifiedBinding | BasicBinding)? BasicBinding
		auto typ = util::pop<ast::BasicBinding>(s);

		if (util::at_node<ast::BasicBinding>(s)) {
			PUSH(QualifiedBinding, util::pop<ast::BasicBinding>(s));
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

	} END;
	RULE(pat_adt) {

	} END;
	RULE(capture_desc) {

	} END;
	RULE(capture) {

	} END;
	RULE(pattern) {

	} END;
	RULE(assign_tuple) {

	} END;
	RULE(assign_pat) {

	} END;


	// Decorators
	RULE(annotation) {
		// stack: bind tuple?
		PUSH(LocalAnnotation, util::pop<ast::BasicBinding>(s));
		// stack: anot
	} END;
	RULE(ganot) {
		// stack: bind tuple?
		PUSH(Annotation, util::pop<ast::BasicBinding>(s));
		// stack: anot
	} END;


	// Control


	// Expressions


	// Statements


	// Binexpr Precedence
	INHERIT(binary_op1, op);
	INHERIT(binary_op2, op);
	INHERIT(binary_op3, op);
	INHERIT(binary_op4, op);
	INHERIT(binary_op5, op);
	INHERIT(binary_op6, op);
	INHERIT(binary_op7, op);


	// Organizational Tagging

}

// Should be around 1300
#undef SENTINEL
#undef INHERIT
#undef TOKEN
#undef END
#undef RULE
#undef PUSH