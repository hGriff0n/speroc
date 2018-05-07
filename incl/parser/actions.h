#pragma once

#include <algorithm>

#include "grammar.h"
#include "interface/CompilationState.h"
#include "util/parser.h"
#include "util/strings.h"

/*
 * Simplify rule definition
 */
#define RULE(gram) \
	template<> struct action<grammar::gram> { \
		template<class Input> \
		static void apply(const Input& in, Stack& s, CompilationState& state)
#define END }
#define LOCATION Location{ in.iterator(), in.input().source() }
#define MAKE(Node, ...) std::make_unique<ast::Node>(__VA_ARGS__, LOCATION)
#define PUSH(Node, ...) s.emplace_back(MAKE(Node, __VA_ARGS__))
#define MAKE_NODE(Node) std::make_unique<ast::Node>(LOCATION)
#define PUSH_NODE(Node) s.emplace_back(MAKE_NODE(Node))
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
	RULE(obrace) { PUSH(Symbol, '{'); } END;
	RULE(obrack) { PUSH(Symbol, '['); } END;
	RULE(oparen) { PUSH(Symbol, '('); } END;
	RULE(ochar) { PUSH(Symbol, '\''); } END;
	RULE(oquote) { PUSH(Symbol, '"'); } END;


	//
	// Keyword Tokens (may remove a couple as it becomes beneficial)
	//
	TOKEN(kas, ast::KeywordType::AS);
	TOKEN(kmut, ast::KeywordType::MUT);
	TOKEN(klet, ast::VisibilityType::PROTECTED);
	TOKEN(kdef, ast::VisibilityType::PUBLIC);
	TOKEN(kpriv, ast::VisibilityType::PRIVATE);
	TOKEN(kwait, ast::KeywordType::WAIT);
	TOKEN(kyield, ast::KeywordType::YIELD);
	TOKEN(kret, ast::KeywordType::RET);
	TOKEN(kcontinue, ast::KeywordType::CONT);
	TOKEN(kbreak, ast::KeywordType::BREAK);
	TOKEN(kfor, ast::KeywordType::FOR);


	//
	// Literals (done)
	//
	RULE(bin_body) {
		if (in.string().size() == 0) {
			state.log(compiler::ID::err, "Missing binary body: `0b` must be followed by a sequence of binary digits <at {}>", LOCATION);
			PUSH_NODE(ValError);
		} else {
			PUSH(Byte, in.string(), 2);
		}
	} END;
	RULE(hex_body) {
		if (in.string().size() == 0) {
			state.log(compiler::ID::err, "Missing hex body: `0x` must be followed by a sequence of hexadecimal digits <at {}>", LOCATION);
			PUSH_NODE(ValError);
		} else {
			PUSH(Byte, in.string(), 16);
		}
	} END;
	RULE(decimal) {
		if (auto str = in.string(); str.find('.') != std::string::npos) {
			PUSH(Float, str);
		} else {
			PUSH(Int, str);
		}
	} END;
	RULE(char_body) {
		PUSH(Char, in.string()[0]);
	} END;
	RULE(_char) {
		// stack: {} char error?
		auto err = POP(CloseSymbolError);
		std::iter_swap(std::rbegin(s), std::rbegin(s) + 1);
		auto sym = POP(Symbol);

		if (err) {
			state.log(compiler::ID::err, "No closing quote found for opening ' <char at {}>", sym->loc);
		}
		// stack: char
	} END;
	RULE(str_body) {
		PUSH(String, util::escape(in.string()));
	} END;
	RULE(string) {
		// stack: {} char error?
		auto err = POP(CloseSymbolError);
		std::iter_swap(std::rbegin(s), std::rbegin(s) + 1);
		auto sym = POP(Symbol);

		if (err) {
			state.log(compiler::ID::err, "No closing quote found for opening \" <string at {}>", sym->loc);
		}
		// stack: char
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
		// stack: symbol stmt* error?
		auto err = POP(CloseSymbolError);
		auto vals = util::popSeq<ast::Statement>(s);

		// Error Handling
		auto sym = POP(Symbol);
		if (err) {
			state.log(compiler::ID::err, "No closing brace found for opening '{}' <scope at {}>", '{', sym->loc);
		}

		PUSH(Block, std::move(vals));
		// stack: block
	} END;
	RULE(missing_texpr) {
		state.log(compiler::ID::err, "Missing tuple item indicated by ',' <tuple at {}>", LOCATION);
		PUSH_NODE(ValError);
	} END;
	RULE(tuple) {
		// stack: symbol valexpr* error?
		auto err = POP(CloseSymbolError);
		auto vals = util::popSeq<ast::ValExpr>(s);

		// Error Handling
		auto sym = POP(Symbol);
		if (err) {
			state.log(compiler::ID::err, "No closing parenthesis found for opening '(' <tuple at {}>", sym->loc);
		}

		PUSH(Tuple, std::move(vals));
		// stack: tuple
	} END;
	RULE(missing_aexpr) {
		state.log(compiler::ID::err, "Missing array item indicated by ',' <array at {}>", LOCATION);
		PUSH_NODE(ValError);
	} END;
	RULE(_array) {
		// stack: {} valexpr*
		auto err = POP(CloseSymbolError);
		auto vals = util::popSeq<ast::ValExpr>(s);

		// Error Handling
		auto sym = POP(Symbol);
		if (err) {
			state.log(compiler::ID::err, "No closing bracket found for opening '[' <array at {}>", sym->loc);
		}

		PUSH(Array, std::move(vals));
		// stack: array
	} END;
	RULE(missing_lexpr) {
		// stack: tuple
		state.log(compiler::ID::err, "Missing function body indicated by '->' <lambda at {}>", s.back()->loc);
		PUSH_NODE(ValError);
		// stack: tuple error
	} END;
	RULE(lambda) {
		// stack: tuple expr
		auto body = POP(ValExpr);
		PUSH(Function, POP(Tuple), std::move(body));
		// stack: fndef
	} END;
	RULE(fwd_dot) {
		PUSH(Future, true);
	} END;
	RULE(missing_dexpr) {
		// stack: future
		state.log(compiler::ID::err, "Missing forwarded function body <dotfn at {}>", POP(Ast)->loc);
		s.pop_back();						// Pop (TODO: ?) from the stack
		PUSH_NODE(ValError);
		// stack: error
	} END;


	//
	// Identifiers
	//
	RULE(typ) {
		PUSH(BasicBinding, in.string(), ast::BindingType::TYPE);
	} END;
	RULE(var) {
		PUSH(BasicBinding, in.string(), ast::BindingType::VARIABLE);

		// TODO: Figure out how to make the short version work
		if (tao::pegtl::parse<grammar::keywords>(tao::pegtl::string_input<>{ in.string(), "" })) {
		//if (tao::pegtl::parse<grammar::keywords>(in)) {
			state.log(compiler::ID::err, "Attempt to use language keyword '{}' as variable <var at {}>", in.string(), LOCATION);
		}
	} END;
	RULE(op) {
		PUSH(BasicBinding, in.string(), ast::BindingType::OPERATOR);
	} END;
	INHERIT(unop, op);
	INHERIT(binop, op);
	RULE(ptyp) {
		PUSH(PathPart, in.string(), ast::BindingType::TYPE);
	} END;
	RULE(pvar) {
		PUSH(PathPart, in.string(), ast::BindingType::VARIABLE);

		if (tao::pegtl::parse<grammar::keywords>(tao::pegtl::string_input<>{ in.string(), "" })) {
		//if (tao::pegtl::parse<grammar::keywords>(in))) {
			state.log(compiler::ID::err, "Attempt to use language keyword '{}' as variable <var at {}>", in.string(), LOCATION);
		}
	} END;
	RULE(path_part) {
		// stack: Path? PathPart array?
		auto gens = POP(Array);
		auto part = POP(PathPart);
		part->gens = std::move(gens);

		if (!util::at_node<ast::Path>(s)) {
			PUSH(Path, std::move(part));
		} else {
			util::view_as<ast::Path>(s.back())->elems.push_back(std::move(part));
		}
		// stack: Path
	} END;
	RULE(typname) {
		// stack: Path
		if (auto* node = util::view_as<ast::Path>(s.back()); node) {
			if (node->elems.back()->type != +ast::BindingType::TYPE) {
				state.log(compiler::ID::err, "Expected type name, found something else <qualtyp at {}>", node->loc);
			}
		}
		// stack: Path
	} END;
	RULE(missing_pat_tup) {
		state.log(compiler::ID::err, "Missing pattern item indicated by ',' <pat_tuple at {}>", LOCATION);
	} END;
	RULE(pat_tuple) {
		// stack: {} pattern* error?
		auto err = POP(CloseSymbolError);
		auto pats = util::popSeq<ast::Pattern>(s);

		// Error Handling
		auto sym = POP(Symbol);
		if (err) {
			state.log(compiler::ID::err, "No closing parenthesis found for opening '(' <pat_tuple at {}>", sym->loc);
		}

		PUSH(TuplePattern, std::move(pats));
		// stack: pattern
	} END;
	RULE(pat_adt) {
		// stack: Path pattern?
		auto tup = POP(TuplePattern);
		PUSH(AdtPattern, POP(Path), std::move(tup));
		// stack: pattern
	} END;
	RULE(pat_missing) {
		// stack: cap?
		if (util::at_node<ast::Token>(s) && util::view_as<ast::Token>(s.back())->holds<ast::CaptureType>()) {
			state.log(compiler::ID::err, "Missing pattern to match specified capture declaration <pattern at {}>", LOCATION);
		} else {
			state.log(compiler::ID::err, "Unexpected Input: Could not match Spero expression \"{}\" <pattern at {}>", in.string(), LOCATION);
		}
		PUSH_NODE(Pattern);
		// stack: Pattern
	} END;
	RULE(capture_desc) {
		// stack: kmut?
		if (in.string().size() == 0) {
			PUSH(Token, ast::CaptureType::NORM);
			return;
		}

		bool is_ref = (in.string()[0] == '&');
		bool is_mut = false;

		if (auto* node = util::view_as<ast::Token>(s.back()); node) {
			is_mut = node->holds<ast::KeywordType>() && node->get<ast::KeywordType>() == +ast::KeywordType::MUT;
		}

		if (is_mut) {
			s.pop_back();
			PUSH(Token, is_ref ? ast::CaptureType::MUTREF : ast::CaptureType::MUT);

		} else if (is_ref) {
			PUSH(Token, ast::CaptureType::REF);
		}
		// stack: cap
	} END;
	RULE(capture) {
		// stack: cap pattern
		auto pat = POP(Pattern);
		pat->cap = POP(Token)->get<ast::CaptureType>();
		s.emplace_back(std::move(pat));
		// stack: pattern
	} END;
	RULE(pat_any) {
		PUSH_NODE(Pattern);
	} END;
	RULE(pat_lit) {
		PUSH(ValPattern, POP(ValExpr));
	} END;
	RULE(resolve_constants) {
		// stack: path
		if (util::at_node<ast::Path>(s)) {
			if (util::view_as<ast::Path>(s.back())->elems.back()->type == +ast::BindingType::TYPE) {
				return action<grammar::pat_adt>::apply(in, s, state);
			}

			auto name = POP(Path);
			if (name->elems.size() > 1) {
				s.emplace_back(std::make_unique<ast::Variable>(std::move(name), name->loc));
				action<grammar::pat_lit>::apply(in, s, state);

			} else {
				s.emplace_back(std::make_unique<ast::VarPattern>(std::move(name), name->loc));
			}
		}
		// stack: patlit | patname
	} END;
	RULE(assign_name) {
		// stack: bind
		PUSH(AssignName, POP(BasicBinding));
		// stack: AssignName
	} END;
	RULE(assign_drop) {
		PUSH_NODE(AssignPattern);
	} END;
	RULE(assign_tuple) {
		// stack: {} asgn_pat* error?
		auto err = POP(CloseSymbolError);
		auto pats = util::popSeq<ast::AssignPattern>(s);

		// Error Handling
		auto sym = POP(Symbol);
		if (err) {
			state.log(compiler::ID::err, "No closing parenthesis found for opening '(' <assign_tuple at {}>", sym->loc);
		}

		PUSH(AssignTuple, std::move(pats));
		// stack: AssignTuple
	} END;
	INHERIT(assign_typ, assign_name);


	//
	// Types
	//
	TOKEN(typ_view, ast::PtrStyling::VIEW);
	TOKEN(typ_ref, ast::PtrStyling::REF);
	TOKEN(typ_ptr, ast::PtrStyling::PTR);
	RULE(single_type) {
		// stack: Path array?
		if (auto gen = POP(Array); gen) {
			PUSH(GenericType, POP(Path), std::move(gen));
		} else {
			PUSH(SourceType, POP(Path));
		}
		// stack: type
	} END;
	RULE(ref_type) {
		// stack: type ptrStyle?
		if (auto tkn = POP(Token); tkn) {
			if (tkn->holds<ast::PtrStyling>()) {
				util::view_as<ast::SourceType>(s.back())->_ptr = tkn->get<ast::PtrStyling>();
			}
		}
		// stack: type
	} END;
	RULE(missing_type) {
		state.log(compiler::ID::err, "Missing type item indicated by ',' <tuple_type at {}>", LOCATION);
	} END;
	RULE(tuple_type) {
		// stack: {} type* error?
		auto err = POP(CloseSymbolError);
		auto types = util::popSeq<ast::Type>(s);

		// Error Handling
		auto sym = POP(Symbol);
		if (err) {
			state.log(compiler::ID::err, "No closing parenthesis found for opening '(' <tuple_type at {}>", sym->loc);
		}

		PUSH(TupleType, std::move(types));
		// stack: tupleType
	} END;
	RULE(fn_type) {
		// stack: TupleType (type | error)
		auto ret = POP(Type);

		// Error handling
		if (!ret) {
			s.pop_back();
			ret = MAKE_NODE(Type);
			state.log(compiler::ID::err, "Missing required return type information <fn_type at {}>", LOCATION);
		}

		if (!util::at_node<ast::TupleType>(s)) {
			PUSH(TupleType, std::deque<ptr<ast::Type>>{});
			state.log(compiler::ID::err, "Missing required argument type information <fn_type at {}>", LOCATION);
		}

		PUSH(FunctionType, POP(TupleType), std::move(ret));
		// stack: FunctionType
	} END;
	RULE(mut_type) {
		// stack: kmut? type
		auto typ = POP(Type);

		if (auto tkn = POP(Token); tkn) {
			if (tkn->holds<ast::KeywordType>()) {
				typ->is_mut = (tkn->get<ast::KeywordType>() == +ast::KeywordType::MUT);
			} else {
				s.push_back(std::move(tkn));
			}
		}

		s.push_back(std::move(typ));
		// stack: type
	} END;
	RULE(braced_type) {
		// stack: {} type error?
		auto err = POP(CloseSymbolError);
		std::iter_swap(std::rbegin(s), std::rbegin(s) + 1);

		// Error handling
		auto sym = POP(Symbol);
		if (err) {
			state.log(compiler::ID::err, "No closing parenthesis found for opening '{}' <braced_type at {}>", '{', sym->loc);
		}
		// stack: type
	} END;
	RULE(and_cont) {
		// stack: (type | and_type) (type | error)
		auto typ = POP(Type);

		if (!typ) {
			s.pop_back();
			state.log(compiler::ID::err, "Missing required type information <and_type at {}>", LOCATION);
		}

		if (!util::at_node<ast::AndType>(s)) {
			PUSH(AndType, POP(Type));
		}

		util::view_as<ast::AndType>(s.back())->elems.push_back(std::move(typ));
		// stack: and_type
	} END;
	RULE(or_cont) {
		// stack: (type | or_type) (type | error)
		auto typ = POP(Type);

		if (!typ) {
			s.pop_back();
			state.log(compiler::ID::err, "Missing required type information <and_type at {}>", LOCATION);
		}

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
		//state.log(compiler::ID::err, "Missing required annotation name <annotation at {}>", LOCATION);
		PUSH(LocalAnnotation, POP(BasicBinding), std::move(tup));
		// stack: anot
	} END;
	RULE(ganot) {
		// stack: bind tuple?
		auto tup = POP(Tuple);
		//state.log(compiler::ID::err, "Missing required annotation name <ganot at {}>", LOCATION);
		PUSH(Annotation, POP(BasicBinding), std::move(tup));
		// stack: anot
	} END;
	RULE(type_inf) {
		// stack: type | error
		if (POP(Error)) {
			state.log(compiler::ID::err, "Missing required type information <type_inf at {}>", LOCATION);
		}
		// stack: type?
	} END;
	RULE(variance) {
		if (in.string().size() == 0) {
			PUSH(Token, ast::VarianceType::INVARIANT);
		} else if (in.string()[0] == '+') {
			PUSH(Token, ast::VarianceType::COVARIANT);
		} else {
			PUSH(Token, ast::VarianceType::CONTRAVARIANT);
		}
	} END;
	TOKEN(variadic, ast::VarianceType::VARIADIC);
	RULE(relation) {
		if (in.string()[0] == ':') {
			PUSH(Token, ast::RelationType::IMPLS);
		} else {
			PUSH(Token, ast::RelationType::NOT_IMPLS);
		}
	} END;
	RULE(error_gentyp) {
		state.log(compiler::ID::err, "Missing required default value <type_gen.def at {}>", LOCATION);
		PUSH_NODE(TypeError);
	} END;
	RULE(default_type) {
		// stack: bind var var? type
		auto def_type = POP(Type);
		auto variadic = POP(Token);
		auto variance = POP(Token);

		if (!variance) {
			std::swap(variance, variadic);
		}

		PUSH(TypeGeneric, POP(BasicBinding), std::move(def_type));
		util::view_as<ast::TypeGeneric>(s.back())->variance = variance->get<ast::VarianceType>();
		util::view_as<ast::TypeGeneric>(s.back())->variadic = (bool)variadic;
		// stack: type_gen
	} END;
	RULE(related_type) {
		// stack: bind var var? (rel (type | error))?
		auto err = POP(Error);
		if (err) {
			state.log(compiler::ID::err, "Missing required type information <type_gen.rel at {}>", LOCATION);
		}

		auto typ = POP(Type);
		auto rel = (typ || err) ? POP(Token)->get<ast::RelationType>() : ast::RelationType::NA;
		auto variadic = POP(Token);
		auto variance = POP(Token);

		// If no variadic was given, then variance will take on "nullptr" due to the stack popping
		if (!variance) {
			std::swap(variance, variadic);
		}

		PUSH(TypeGeneric, POP(BasicBinding), std::move(typ), rel, variance->get<ast::VarianceType>(), (bool)variadic);
		// stack: type_gen
	} END;
	RULE(error_genval) {
		state.log(compiler::ID::err, "Missing required default value <val_gen.def at {}>", LOCATION);
		PUSH_NODE(ValError);
	} END;
	RULE(related_val) {
		// stack: bind (rel (type | error))?
		auto err = POP(Error);
		if (err) {
			state.log(compiler::ID::err, "Missing required type information <val_gen.rel at {}>", LOCATION);
		}

		auto typ = POP(Type);
		auto rel = (typ || err) ? POP(Token)->get<ast::RelationType>() : ast::RelationType::NA;

		PUSH(ValueGeneric, POP(BasicBinding), std::move(typ), rel);
		// stack: val_gen
	} END;
	RULE(default_val) {
		// stack: bind (val | error)
		auto val = POP(ValExpr);
		PUSH(ValueGeneric, POP(BasicBinding), std::move(val));
		// stack: val_gen
	} END;	
	RULE(lit_gen) {
		// stack: lit
		if (auto lit = POP(ValExpr); lit) {
			PUSH(LitGeneric, std::move(lit));
		}
		// stack:
	} END;
	RULE(gen_parterror) {
		state.log(compiler::ID::err, "Unexpected input: Could not match spero generic \"{}\" <gen_part {}>", in.string(), LOCATION);
	} END;
	RULE(_generic) {
		// stack: {} gen_part* error?
		auto err = POP(CloseSymbolError);
		auto parts = util::popSeq<ast::GenericPart>(s);

		// Error Handling
		auto sym = POP(Symbol);
		if (err) {
			state.log(compiler::ID::err, "No closing parenthesis found for opening '[' <generic at {}>", sym->loc);
		}

		PUSH(GenericArray, std::move(parts));
		// stack: gentype
	} END;
	RULE(adt) {
		// stack: typ sym? type?
		auto typ_args = POP(TupleType);
		
		if (POP(Symbol)) {
			state.log(compiler::ID::err, "Missing expected tuple types indicated by opening '(' <adt at {}>", s.back()->loc);
		}

		PUSH(Adt, POP(BasicBinding), std::move(typ_args));
		// stack: adt
	} END;
	RULE(error_adt) {
		state.log(compiler::ID::err, "Missing expected ADT constructor indicated by '|' <type at {}>", LOCATION);
	} END;
	SENTINEL(arg_sentinel);
	RULE(arg_inf) {
		action<grammar::type_inf>::apply(in, s, state);
		s.pop_back();
	} END;
	RULE(arg) {
		// stack: bind type?
		auto typ = POP(Type);
		if (typ) {
			s.pop_back();
		}
		PUSH(Argument, POP(BasicBinding), std::move(typ));
		// stack: arg
	} END;
	RULE(missing_arg) {
		state.log(compiler::ID::err, "Missing arg item indicated by ',' <arg_tuple at {}>", LOCATION);
	} END;
	RULE(arg_tuple) {
		// stack: {} arg* error?
		auto err = POP(CloseSymbolError);
		auto args = util::popSeq<ast::Argument>(s);

		// Error Handling
		auto sym = POP(Symbol);
		if (err) {
			state.log(compiler::ID::err, "No closing parenthesis found for opening '(' <arg_tuple at {}>", sym->loc);
		}

		PUSH(ArgTuple, std::move(args));
		// stack: argtuple
	} END;


	//
	// Control
	//
	RULE(missing_in) {
		// stack: "for" pattern | error
		POP(Pattern);
		auto key = POP(Token);
		state.log(compiler::ID::err, "Missing keyword `in` <for loop at {}>", key->loc);
		PUSH_NODE(ValError);
		// stack: 
	} END;
	RULE(missing_gen) {
		// stack: "for" _
		auto loc = (*(s.rbegin() + 1))->loc;
		state.log(compiler::ID::err, "Missing loop generator expression <for at {}>", loc);
		PUSH_NODE(ValError);
		// stack: "for" _ error
	} END;
	RULE(missing_fbody) {
		// stack: "for" _ _
		auto loc = (*(s.rbegin() + 2))->loc;
		state.log(compiler::ID::err, "Missing loop body expression <for at {}>", loc);
		PUSH_NODE(ValError);
		// stack: "for" _ _ error
	} END;
	RULE(missing_pat) {
		// stack: "for" _
		POP(Token);						// Pop extraneous token from failed capture_desc
		auto loc = POP(Token)->loc;
		state.log(compiler::ID::err, "Missing decomoposition pattern <for at {}>", loc);
		PUSH_NODE(ValError);			// Push ValError to represent failed 'for' parsing (grammar skips the rest if this action is run)
		// stack: error
	} END;
	RULE(infor) {
		// stack: "for" pattern (gen | error) (body | error)
		auto body = POP(ValExpr);
		auto generator = POP(ValExpr);
		auto pat = POP(Pattern);
		POP(Token);

		// Push the completed for loop only if every sub-part completed successfully
		if (!util::is_type<ast::ValError>(body) && !util::is_type<ast::ValError>(generator)) {
			PUSH(For, std::move(pat), std::move(generator), std::move(body));
		} else {
			PUSH_NODE(ValError);
		}
		// stack: for | error
	} END;
	RULE(missing_wtest) {
		// stack: 
		state.log(compiler::ID::err, "Missing loop test expression <while at {}>", LOCATION);
		PUSH_NODE(ValError);
		// stack: error
	} END;
	RULE(missing_wbody) {
		// stack: _
		auto loc = (*s.rbegin())->loc;
		state.log(compiler::ID::err, "Missing loop body expression <while at {}>", loc);
		PUSH_NODE(ValError);
		// stack: _ error
	} END;
	RULE(whilel) {
		// stack: (test | error) (body | error)
		auto body = POP(ValExpr);
		auto test = POP(ValExpr);

		if (!util::is_type<ast::ValError>(body) && !util::is_type<ast::ValError>(test)) {
			PUSH(While, std::move(test), std::move(body));
		} else {
			PUSH_NODE(ValError);
		}
		// stack: loop | error
	} END;
	RULE(missing_lbody) {
		// stack:
		// TODO: Push "loop" token on stack so that I can extract the correct location information
		state.log(compiler::ID::err, "Missing loop body expression <loop at {}>", LOCATION);
		PUSH_NODE(ValError);
		// stack: error
	} END;
	RULE(loop) {
		// stack: valexpr | error
		if (!util::at_node<ast::ValError>(s)) {
			PUSH(Loop, POP(ValExpr));
		}
		// stack: loop | error
	} END;
	RULE(elseif_key) {
		state.log(compiler::ID::err, "Invalid keyword: Use 'elsif' instead <branch at {}>", LOCATION);
	} END;
	RULE(missing_eitest) {
		state.log(compiler::ID::err, "Missing branch test expression <elsif at {}>", LOCATION);
		PUSH_NODE(ValError);
	} END;
	RULE(missing_eibody) {
		state.log(compiler::ID::err, "Missing branch body expression <elsif at {}>", LOCATION);
		PUSH_NODE(ValError);
	} END;
	RULE(elsif_case) {
		// stack: IfBranch test body
		auto body = POP(ValExpr);
		PUSH(IfBranch, POP(ValExpr), std::move(body), true);
		// stack: IfBranch IfBranch
	} END;
	RULE(missing_itest) {
		state.log(compiler::ID::err, "Missing branch test expression <if at {}>", LOCATION);
		PUSH_NODE(ValError);
	} END;
	RULE(missing_ibody) {
		state.log(compiler::ID::err, "Missing branch body expression <if at {}>", LOCATION);
		PUSH_NODE(ValError);
	} END;
	RULE(if_branch) {
		// stack: (test | error) (body | error)
		auto body = POP(ValExpr);
		PUSH(IfBranch, POP(ValExpr), std::move(body), false);
		// stack: IfBranch
	} END;
	RULE(missing_ebody) {
		state.log(compiler::ID::err, "Missing branch body expression <else at {}>", LOCATION);
		PUSH_NODE(ValError);
	} END;
	RULE(branch) {
		// stack: IfBranch* (valexpr | error)?
		auto _else_ = (util::at_node<ast::IfBranch>(s) ? nullptr : POP(ValExpr));
		auto ifs = util::popSeq<ast::IfBranch>(s);
		PUSH(IfElse, std::move(ifs), std::move(_else_));
		// stack: IfElse
	} END;
	RULE(missing_arrow) {
		// stack: pat _
		// TODO: Find the pattern, not just the if_core
		auto loc = (*s.rbegin())->loc;
		state.log(compiler::ID::err, "Missing arrow keyword ('=>') <case at {}>", loc);
		// stack: pat _
	} END;
	RULE(case_pat) {
		// stack: <T> pattern*
		auto pats = util::popSeq<ast::Pattern>(s);
		PUSH(TuplePattern, std::move(pats));
		// stack: <T> pattern
	} END;
	RULE(missing_cbody) {
		state.log(compiler::ID::err, "Missing case body expression <case at {}>", LOCATION);
		PUSH_NODE(ValError);
	} END;
	RULE(missing_cpat) {
		// stack: _
		auto loc = POP(Token)->loc;
		state.log(compiler::ID::err, "Missing continued pattern indicated by ',' <case at {}>", loc);
		// stack:
	} END;
	RULE(_case) {
		// stack: pattern valexpr? (body | error)
		auto expr = POP(ValExpr);
		auto if_guard = POP(ValExpr);
		PUSH(Case, POP(TuplePattern), std::move(if_guard), std::move(expr));
		// stack: case
	} END;
	RULE(missing_mexpr) {
		// stack: 
		state.log(compiler::ID::err, "Missing match switch expression <match at {}>", LOCATION);
		PUSH_NODE(ValError);
		PUSH(Symbol, '{');				// Push symbol on the stack for reporting of match errors
		// stack: error
	} END;
	RULE(missing_brace) {
		// stack: _
		state.log(compiler::ID::err, "Missing opening brace '{}' <match at {}>", '{', s.back()->loc);
		PUSH(Symbol, '{');
		// stack: _ error
	} END;
	RULE(matchs) {
		// stack: valexpr {} case+ keyword error?
		auto err = POP(CloseSymbolError);
		POP(Token);
		auto cases = util::popSeq<ast::Case>(s);

		// Error Handling
		auto sym = POP(Symbol);
		if (err) {
			state.log(compiler::ID::err, "No closing brace found for opening '{}' <match at {}>", '{', sym->loc);
		}
		if (cases.size() == 0) {
			state.log(compiler::ID::err, "Missing match case expressions <match at {}>", sym->loc);
		}

		PUSH(Match, POP(ValExpr), std::move(cases));
		// stack: match
	} END;
	RULE(jump) {
		// stack: kwd expr?
		auto expr = POP(ValExpr);
		PUSH(Jump, POP(Token)->get<ast::KeywordType>(), std::move(expr));
		// stack: jump
	} END;
	INHERIT(dotloop, loop);
	RULE(dotwhile) {
		// stack: body (test | error)
		auto test = POP(ValExpr);

		if (!util::is_type<ast::ValError>(test)) {
			PUSH(While, std::move(test), POP(ValExpr));
		} else {
			s.pop_back();
			PUSH_NODE(ValError);
		}
		// stack: while
	} END;
	RULE(missing_dotpat) {
		// stack: body "for" _
		POP(Token);						// Pop off extraneous token from capture_desc
		POP(Token);
		auto loc = POP(ValExpr)->loc;
		state.log(compiler::ID::err, "Missing decomoposition pattern <for at {}>", loc);
		PUSH_NODE(ValError);
		// stack: error
	} END;
	RULE(dot_infor) {
		// stack: body "for" pattern (gen | error)
		auto generator = POP(ValExpr);
		auto pattern = POP(Pattern);
		POP(Token);

		if (!util::is_type<ast::ValError>(generator)) {
			PUSH(For, std::move(pattern), std::move(generator), POP(ValExpr));
		} else {
			s.pop_back();
			PUSH_NODE(ValError);
		}
		// stack: for
	} END;
	RULE(dotif) {
		// stack: body test
		auto test = POP(ValExpr);
		PUSH(IfBranch, std::move(test), POP(ValExpr), false);
		// stack: IfBranch
	} END;
	INHERIT(dotbranch, branch);
	INHERIT(dotmatch, matchs);
	RULE(dotjump) {
		// stack: expr kwd
		auto tkn = POP(Token)->get<ast::KeywordType>();
		PUSH(Jump, tkn, POP(ValExpr));
		// stack: jump
	} END;


	//
	// Expressions
	//
	RULE(call) {
		// stack: expr tup
		auto tup = POP(Tuple);
		PUSH(FnCall, POP(ValExpr), std::move(tup));
		// stack: fncall
	} END;
	RULE(pathed_var) {
		// stack: path | var
		if (util::at_node<ast::Path>(s)) {
			auto loc = s.back()->loc;
			s.emplace_back(std::make_unique<ast::Variable>(POP(Path), loc));
		}
		// stack: var
	} END;
	RULE(op_var) {
		// stack: binding
		auto bind = POP(BasicBinding);
		s.emplace_back(std::make_unique<ast::Path>(std::make_unique<ast::PathPart>(bind->name, ast::BindingType::OPERATOR, bind->loc), bind->loc));
		action<grammar::pathed_var>::apply(in, s, state);
		// stack: var
	} END;
	RULE(type_const_tail) {
		// stack: var tuple? block?
		auto body = POP(Block);
		auto args = POP(Tuple);

		if (body) {
			auto var = POP(Variable);
			PUSH(TypeExtension, std::move(var->name), std::move(args), std::move(body));
		} else if (args) {
			PUSH(FnCall, POP(Variable), std::move(args));
		}
		// stack: FnCall | Var | TypeExt
	} END;
	RULE(indexeps) {
		// stack: expr
		s.emplace_back(nullptr);
		std::iter_swap(std::rbegin(s), std::rbegin(s) + 1);
		// stack: {} expr
	} END;
	RULE(missing_fncall) {
		state.log(compiler::ID::err, "Missing fncall indicated by opening '.' <index at {}>", s.back()->loc);
		PUSH_NODE(ValError);
	} END;
	RULE(index_cont) {
		// stack: {} expr*
		auto exprs = util::popSeq<ast::ValExpr>(s);
		s.pop_back();
		PUSH(Index, std::move(exprs));
		// stack: index
	} END;
	RULE(missing_index) {
		auto val = POP(ValExpr);
		s.pop_back();						// Pop leftover (TODO: ?) from the stack
		state.log(compiler::ID::err, "Missing index/dot_ctrl indicated by opening '.' <index at {}>", val->loc);
		s.emplace_back(std::move(val));
	} END;
	RULE(unopcall) {
		// stack: bind expr
		auto expr = POP(ValExpr);
		auto bind = POP(BasicBinding);

		PUSH(UnOpCall, std::move(expr), std::move(bind));
		// stack: unexpr
	} END;
	RULE(missing_unexpr) {
		// stack: bind
		state.log(compiler::ID::err, "Missing valexpr body indicated by unary operator <unop at {}>", s.back()->loc);
		PUSH_NODE(ValError);
		// stack: bind error
	} END;


	//
	// Statements
	//
	RULE(missing_module) {
		state.log(compiler::ID::err, "Missing module name indicated by 'mod' keyword <mod at {}>", LOCATION);
		PUSH_NODE(ValError);
	} END;
	RULE(_mod_dec) {
		// stack: Path
		auto path = POP(Path);
		
		for (auto&& part : path->elems) {
			if (part->type != +ast::BindingType::VARIABLE || part->gens != nullptr) {
				state.log(compiler::ID::err, "Module path parts must be basic vars <at {}>", part->loc);
			}
		}

		PUSH(ModDec, std::move(path));
		// stack: ModDec
	} END;
	RULE(missing_fortype) {
		state.log(compiler::ID::err, "Missing implementor type <impl at {}>", s.back()->loc);
		PUSH_NODE(TypeError);
	} END;
	TOKEN(for_type, ast::KeywordType::FOR);
	RULE(missing_impltype) {
		state.log(compiler::ID::err, "Missing implementation type <impl at {}>", LOCATION);
		PUSH_NODE(TypeError);
	} END;
	RULE(missing_impldef) {
		state.log(compiler::ID::err, "Missing implementation body <impl at {}>", s.back()->loc);
		PUSH_NODE(ScopeError);
	} END;
	RULE(impl) {
		// stack: type (type "for")? scope
		auto block = POP(Block);
		auto for_type = (POP(Token) ? POP(SourceType) : nullptr);
		PUSH(ImplExpr, std::move(for_type), POP(SourceType), std::move(block));
		// stack: impl
	} END;
	RULE(mul_imp) {
		// stack: path {} PathPart* error?
		auto err = POP(CloseSymbolError);
		auto bindings = util::popSeq<ast::PathPart>(s);

		// Error Handling
		auto sym = POP(Symbol);
		if (err) {
			state.log(compiler::ID::err, "No closing brace found for opening '{}' <mul_imp at {}>", '{', sym->loc);
		}

		PUSH(MultipleImport, POP(Path), std::move(bindings));
		// stack: MultipleImport
	} END;
	RULE(rebind) {
		// stack: Path {} Path
		auto new_name = POP(Path);
		s.pop_back();
		auto old_name = POP(Path);

		if (old_name->elems.back()->type != new_name->elems.back()->type) {
			state.log(compiler::ID::err, "Attempt to rebind a {} as a {} <at {}>", old_name->elems.back()->type._to_string(), new_name->elems.back()->type._to_string(), LOCATION);
		}

		PUSH(Rebind, std::move(old_name), std::move(new_name));
		// stack: ModRebind
	} END;
	RULE(err_rebind) {
		// stack: Path {}
		POP(Token);
		auto loc = util::view_as<ast::Path>(s.back())->loc;

		state.log(compiler::ID::err, "Cannot rebind symbol as `{}` <at {}>", *util::view_as<ast::Path>(s.back()), loc);
		PUSH(SingleImport, POP(Path));
		// stack: Path
	} END;
	RULE(missing_import) {
		state.log(compiler::ID::err, "Missing import binding indicated by keyword 'use' <use at {}>", LOCATION);
		PUSH_NODE(ValError);
	} END;
	RULE(import_single) {
		// stack: Path
		PUSH(SingleImport, POP(Path));
		// stack: ModRebindImport
	} END;
	RULE(missing_typedef) {
		// stack: vis pat gen? mut? cons*
		state.log(compiler::ID::err, "Missing type definition body <type_assign at {}>", s.back()->loc);
		PUSH_NODE(ScopeError);
		// stack: vis pat gen? mut? cons* error
	} END;
	RULE(missing_type_assign) {
		// stack: vis pat gen?
		state.log(compiler::ID::err, "Missing assignment operator '=' <type_assign at {}>", s.back()->loc);
		PUSH_NODE(ScopeError);
		// stack: vis pat gen? error
	} END;
	RULE(type_assign) {
		// stack: vis pat gen? "mut"? cons* scope
		auto body = POP(Block);
		auto cons = util::popSeq<ast::Constructor>(s);
		auto mut = POP(Token) != nullptr;
		auto gen = POP(GenericArray);
		auto pat = POP(AssignPattern);

		PUSH(TypeAssign,
			POP(Token)->get<ast::VisibilityType>(),
			std::move(pat), std::move(gen), mut,
			std::move(cons), std::move(body));
		// stack: TypeAssign
	} END;
	RULE(asgn_val) {
		// stack: (vis pat gen? type? val) | in
		if (!util::at_node<ast::InAssign>(s)) {
			auto val = POP(ValExpr);
			auto typ = POP(Type);
			auto gen = POP(GenericArray);
			auto pat = POP(AssignPattern);

			PUSH(VarAssign,
				POP(Token)->get<ast::VisibilityType>(),
				std::move(pat), std::move(gen),
				std::move(typ), std::move(val));
		}
		// stack: VarAssign | in
	} END;
	RULE(missing_invexpr) {
		// stack: vis pat gen? type? val
		state.log(compiler::ID::err, "Missing bound scoped expression <in_asign at {}>", s.back()->loc);
		PUSH_NODE(ValError);
		// stack: vis pat gen? type? val error
	} END;
	RULE(missing_asexpr) {
		// stack: vis pat gen? type?
		state.log(compiler::ID::err, "Missing assignment value expression <in_asign at {}>", s.back()->loc);
		PUSH_NODE(ValError);
		PUSH_NODE(ValError);
		// stack: vis pat gen? type? error error
	} END;
	RULE(missing_inexpr) {
		// stack: vis pat gen? type? val
		state.log(compiler::ID::err, "Missing scoping operator 'in' <in_asign at {}>", s.back()->loc);
		PUSH_NODE(ValError);
		// stack: vis pat gen? type? val error
	} END;
	RULE(missing_assignment) {
		// stack: vis pat gen? type?
		state.log(compiler::ID::err, "Missing assignment operator '=' <in_asign at {}>", s.back()->loc);
		PUSH_NODE(ValError);
		PUSH_NODE(ValError);
		// stack: vis pat gen? type? error error
	} END;
	RULE(missing_aspat) {
		// stack: vis
		state.log(compiler::ID::err, "Missing decomoposition pattern <in_asign at {}>", POP(Token)->loc);
		PUSH_NODE(ValError);
		// stack: error
	} END;
	RULE(_in_assign) {
		// stack: vis pat gen? type? val expr
		auto context = POP(ValExpr);
		action<grammar::asgn_val>::apply(in, s, state);
		PUSH(InAssign, POP(VarAssign), std::move(context));
		// stack: InAssign
	} END;
	INHERIT(asgn_in, _in_assign);
	RULE(_interface) {
		// stack: (vis pat gen? type) | varasgn
		if (!util::at_node<ast::VarAssign>(s)) {
			auto typ = POP(Type);
			auto gen = POP(GenericArray);
			auto pat = POP(AssignPattern);
			PUSH(Interface,
				POP(Token)->get<ast::VisibilityType>(),
				std::move(pat), std::move(gen), std::move(typ));
		}
		// stack: Interface | VarAssign
	} END;
	RULE(missing_val_assign) {
		// stack:
		state.log(compiler::ID::err, "Missing assignment value expression <val_assign at {}>", s.back()->loc);
		PUSH_NODE(ValError);
		// stack: error
	} END;
	RULE(missing_interface) {
		// stack: vis pat gen?
		POP(GenericArray);
		POP(AssignPattern);
		state.log(compiler::ID::err, "Missing interface/assignment indicated by varname <val_assign at {}>", POP(Token)->loc);
		PUSH_NODE(ValError);
		// stack: error
	} END;
	RULE(standalone_visibility) {
		// stack: vis
		state.log(compiler::ID::err, "Missing definition indicated by visibility <assign at {}>", POP(Token)->loc);
		PUSH_NODE(ValError);
		// stack: error
	} END;


	// Binexpr Precedence
	INHERIT(binary_op1, op);
	INHERIT(binary_op2, op);
	INHERIT(binary_op3, op);
	INHERIT(binary_op4, op);
	INHERIT(binary_op5, op);
	INHERIT(binary_op6, op);
	INHERIT(binary_op7, op);
	RULE(binary_cont1) {
		// stack: valexpr op valexpr?
		auto rhs = POP(ValExpr);
		auto op = POP(BasicBinding);

		if (!rhs) {
			rhs = std::make_unique<ast::Future>(true, op->loc);
		}
		
		PUSH(BinOpCall, POP(ValExpr), std::move(rhs), op->name);
		// stack: binexpr
	} END;
	INHERIT(binary_cont2, binary_cont1);
	INHERIT(binary_cont3, binary_cont1);
	INHERIT(binary_cont4, binary_cont1);
	INHERIT(binary_cont5, binary_cont1);
	INHERIT(binary_cont6, binary_cont1);
	INHERIT(binary_cont7, binary_cont1);


	// Organizational Tagging
	RULE(missing_valexpr) {
		state.log(compiler::ID::err, "Missing valexpr expression indicated by 'mut' keyword <valexpr at {}>", POP(Token)->loc);
		PUSH_NODE(ValError);
	} END;
	RULE(missing_valexpr2) {
		POP(Token);
		state.log(compiler::ID::err, "Missing valexpr expression indicated by 'do' keyword <valexpr at {}>", LOCATION);
		PUSH_NODE(ValError);
	} END;
	RULE(valexpr) {
		// stack: kmut? expr type?
		auto inf = POP(Type);
		auto expr = POP(ValExpr);
		auto mut = POP(Token);

		expr->type = std::move(inf);
		expr->is_mut = (bool)mut;
		s.emplace_back(std::move(expr));
		// stack: expr
	} END;
	RULE(missing_stmt) {
		// stack:
		state.log(compiler::ID::err, "Statement Not Found: Could not bind local annotation to statement <stmt at {}>", LOCATION);
		// TODO: Replace with StmtError?
		PUSH_NODE(ValError);
		// stack: error
	} END;
	RULE(statement) {
		// stack: annotation* stmt
		auto stmt = POP(Statement);
		auto anots = util::popSeq<ast::LocalAnnotation>(s);

		stmt->annots = std::move(anots);
		s.emplace_back(std::move(stmt));
		// stack: stmt
	} END;


	// Errors
	RULE(errorparen) {
		PUSH_NODE(CloseSymbolError);
	} END;
	INHERIT(errorbrack, errorparen);
	INHERIT(errorbrace, errorparen);
	INHERIT(errorchar, errorparen);
	INHERIT(errorquote, errorparen);
	RULE(errortype) {
		PUSH_NODE(Error);
	} END;
	RULE(leftovers) {
		state.log(compiler::ID::err, "Unexpected input: Could not match spero expression \"{}\" <at {}>", in.string(), LOCATION);
	} END;

}

// Should be around 1300
#undef SENTINEL
#undef INHERIT
#undef TOKEN
#undef END
#undef RULE
#undef PUSH
#undef POP