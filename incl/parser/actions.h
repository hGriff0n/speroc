#pragma once

#include "grammar.h"
#include "interface/CompilationState.h"
#include "util/parser.h"

#include <algorithm>

/*
 * Simplify rule definition
 */
#define RULE(gram) \
	template<> struct action<grammar::gram> { \
		template<class Input> \
		static void apply(const Input& in, Stack& s, CompilationState& state)
#define END }
#define MAKE(Node, ...) std::make_unique<ast::Node>(__VA_ARGS__, in.position())
#define PUSH(Node, ...) s.emplace_back(MAKE(Node, __VA_ARGS__))
#define MAKE_NODE(Node) std::make_unique<ast::Node>(in.position())
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
	SENTINEL(obrace);
	SENTINEL(obrack);
	SENTINEL(oparen);


	//
	// Keyword Tokens (may remove a couple as it becomes beneficial)
	//
	TOKEN(kmut, ast::KeywordType::MUT);
	TOKEN(klet, ast::VisibilityType::PROTECTED);
	TOKEN(kdef, ast::VisibilityType::PUBLIC);
	TOKEN(kpriv, ast::VisibilityType::PRIVATE);
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
	RULE(fwd_dot) {
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
	INHERIT(unop, op);
	INHERIT(binop, op);
	RULE(mod_path) {
		// stack: (BasicBinding | QualifiedBinding) BasicBinding
		auto part = POP(BasicBinding);

		if (util::at_node<ast::BasicBinding>(s)) {
			PUSH(QualifiedBinding, POP(BasicBinding));
		}

		util::view_as<ast::QualifiedBinding>(s.back())->elems.push_back(std::move(part));
		// stack: QualifiedBinding
	} END;
	INHERIT(type_path, mod_path);
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
	RULE(typname) {
		// stack: qualbind | basicbind
		if (util::at_node<ast::BasicBinding>(s)) {
			PUSH(QualifiedBinding, POP(BasicBinding));
		}
		// stack: qualbind
	} END;
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

		// Swap around the cap and the binding to remove the duplicate binding
		std::iter_swap(std::rbegin(s), std::rbegin(s) + 1);
		s.pop_back();

		PUSH(VarPattern, MAKE(QualifiedBinding, std::move(var)));
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
		if (in.string().size() == 0) {
			PUSH(Token, ast::CaptureType::NORM);
			return;
		}

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
	RULE(assign_name) {
		// stack: bind
		PUSH(AssignName, POP(BasicBinding));
		// stack: AssignName
	} END;
	RULE(assign_drop) {
		PUSH_NODE(AssignPattern);
	} END;
	RULE(assign_tuple) {
		// stack: {} asgn_pat*
		auto pats = util::popSeq<ast::AssignPattern>(s);
		s.pop_back();
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
	RULE(type_gen) {
		// stack: bind var var? (rel type)?
		auto typ = POP(Type);
		auto rel = (typ ? POP(Token)->get<ast::RelationType>() : ast::RelationType::NA);
		auto variadic = POP(Token);
		auto variance = POP(Token);

		// If no variadic was given, then variance will take on "nullptr" due to the stack popping
		if (!variance) {
			std::swap(variance, variadic);
		}

		PUSH(TypeGeneric, POP(BasicBinding), std::move(typ), rel, variance->get<ast::VarianceType>(), (bool)variadic);
		// stack: type_gen
	} END;
	RULE(val_gen) {
		// _stack: bind (rel type)? expr?
		// stack: bind (rel type)?
		auto typ = POP(Type);
		auto rel = (typ ? POP(Token)->get<ast::RelationType>() : ast::RelationType::NA);

		PUSH(ValueGeneric, POP(BasicBinding), std::move(typ), rel);
		// stack: val_gen
	} END;
	RULE(_generic) {
		// stack: {} gen_part*
		auto parts = util::popSeq<ast::GenericPart>(s);
		s.pop_back();
		PUSH(GenericArray, std::move(parts));
		// stack: gentype
	} END;
	RULE(adt) {
		// stack: typ type?
		auto typ_args = POP(TupleType);
		PUSH(Adt, POP(BasicBinding), std::move(typ_args));
		// stack: adt
	} END;
	SENTINEL(arg_sentinel);
	RULE(arg) {
		// stack: bind type?
		auto typ = POP(Type);
		if (typ) {
			s.pop_back();
		}
		PUSH(Argument, POP(BasicBinding), std::move(typ));
		// stack: arg
	} END;
	RULE(arg_tuple) {
		// stack: {} arg*
		auto args = util::popSeq<ast::Argument>(s);
		s.pop_back();
		PUSH(ArgTuple, std::move(args));
		// stack: argtuple
	} END;


	//
	// Control
	//
	RULE(forl) {
		// stack: pattern gen body
		auto body = POP(ValExpr);
		auto generator = POP(ValExpr);
		PUSH(For, POP(Pattern), std::move(generator), std::move(body));
		// stack: for
	} END;
	RULE(whilel) {
		// stack: test body
		auto body = POP(ValExpr);
		PUSH(While, POP(ValExpr), std::move(body));
		// stack: while
	} END;
	RULE(loop) {
		// stack: valexpr
		PUSH(Loop, POP(ValExpr));
		// stack: loop
	} END;
	RULE(elsif_case) {
		// stack: IfBranch test body
		auto body = POP(ValExpr);
		PUSH(IfBranch, POP(ValExpr), std::move(body), true);
		// stack: IfBranch IfBranch
	} END;
	RULE(if_branch) {
		// stack: test body
		auto body = POP(ValExpr);
		PUSH(IfBranch, POP(ValExpr), std::move(body), false);
		// stack: IfBranch
	} END;
	RULE(branch) {
		// stack: IfBranch* valexpr?
		auto _else_ = (util::at_node<ast::IfBranch>(s) ? nullptr : POP(ValExpr));
		auto ifs = util::popSeq<ast::IfBranch>(s);
		PUSH(IfElse, std::move(ifs), std::move(_else_));
		// stack: IfElse
	} END;
	RULE(case_pat) {
		// stack: <T> pattern*
		auto pats = util::popSeq<ast::Pattern>(s);
		PUSH(TuplePattern, std::move(pats));
		// stack: <T> pattern
	} END;
	RULE(_case) {
		// stack: pattern valexpr? body
		auto expr = POP(ValExpr);
		auto if_guard = POP(ValExpr);
		PUSH(Case, POP(TuplePattern), std::move(if_guard), std::move(expr));
		// stack: case
	} END;
	RULE(matchs) {
		// stack: valexpr {} case+ keyword
		POP(Token);
		auto cases = util::popSeq<ast::Case>(s);
		s.pop_back();
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
		// stack: body test
		auto test = POP(ValExpr);
		PUSH(While, std::move(test), POP(ValExpr));
		// stack: while
	} END;
	RULE(dotfor) {
		// stack: body pattern gen
		auto generator = POP(ValExpr);
		auto pattern = POP(Pattern);
		PUSH(For, std::move(pattern), std::move(generator), POP(ValExpr));
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
	RULE(vareps) {
		// stack: basicbind | qualbind array
		auto inst = POP(Array);
		if (util::at_node<ast::BasicBinding>(s)) {
			PUSH(Variable, MAKE(QualifiedBinding, POP(BasicBinding)), std::move(inst));
		} else {
			PUSH(Variable, POP(QualifiedBinding), std::move(inst));
		}
		// stack: variable
	} END;
	INHERIT(op_var, vareps);
	RULE(type_var) {
		// stack: qualbind array?
		auto inst = POP(Array);
		PUSH(Variable, POP(QualifiedBinding), std::move(inst));
		// stack: variable
	} END;
	RULE(type_const) {
		// stack: variable tuple? block?
		auto body = POP(Block);
		auto args = POP(Tuple);

		if (body) {
			auto var = POP(Variable);
			 PUSH(TypeExtension, std::move(var->name), std::move(var->inst_args), std::move(args), std::move(body));
		} else if (args) {
			PUSH(FnCall, POP(Variable),std::move(args));
		}
		// stack: FnCall | Var | TypeExt
	} END;
	RULE(raw_const) {
		// stack: basicbind array? tuple? block?
		auto body = POP(Block);
		auto args = POP(Tuple);

		action<grammar::vareps>::apply(in, s, state);
		if (body) {
			auto var = POP(Variable);
			PUSH(TypeExtension, std::move(var->name), std::move(var->inst_args), std::move(args), std::move(body));

		} else if (args) {
			PUSH(FnCall, POP(Variable), std::move(args));
		}
		// stack: FnCall | Var | TypeExt
	} END;
	RULE(var_val) {
		// stack: variable array?
		auto inst = POP(Array);
		if (inst) {
			util::view_as<ast::Variable>(s.back())->inst_args = std::move(inst);
		}
		// stack: variable
	} END;

	RULE(indexeps) {
		// stack: expr
		s.emplace_back(nullptr);
		std::iter_swap(std::rbegin(s), std::rbegin(s) + 1);
		// stack: {} expr
	} END;
	RULE(index_cont) {
		// stack: {} expr*
		auto exprs = util::popSeq<ast::ValExpr>(s);
		s.pop_back();
		PUSH(Index, std::move(exprs));
		// stack: index
	} END;
	RULE(unopcall) {
		// stack: bind expr
		auto expr = POP(ValExpr);
		auto bind = POP(BasicBinding);

		PUSH(UnOpCall, std::move(expr), std::move(bind));
		// stack: unexpr
	} END;


	//
	// Statements
	//
	RULE(mod_dec) {
		// stack: basicbind | qualbind
		if (util::at_node<ast::BasicBinding>(s)) {
			PUSH(QualifiedBinding, POP(BasicBinding));
		}

		PUSH(ModDec, POP(QualifiedBinding));
		// stack: ModDec
	} END;
	TOKEN(for_type, ast::KeywordType::FOR);
	RULE(impl) {
		// stack: type (type "for")? scope
		auto block = POP(Block);
		auto for_type = (POP(Token) ? POP(SourceType) : nullptr);
		PUSH(ImplExpr, std::move(for_type), POP(SourceType), std::move(block));
		// stack: impl
	} END;
	RULE(mul_imp) {
		// stack: qualbind {} bind*
		auto bindings = util::popSeq<ast::BasicBinding>(s);
		s.pop_back();
		PUSH(MultipleImport, POP(QualifiedBinding), std::move(bindings));
		// stack: MultipleImport
	} END;
	RULE(alieps) {
		// stack: qualbind bind?
		auto bind = POP(BasicBinding);

		// If 'bind' is a var, then mod_path/imps gobbles it into the qualified binding
		// This restores the division to match our expected/desired behavior
		if (!bind) {
			auto mod = POP(QualifiedBinding);
			bind = std::move(mod->elems.back());
			mod->elems.pop_back();

			if (mod->elems.size() == 0) { mod = nullptr; }
			s.emplace_back(std::move(mod));

		// If there is no module on the stack at all, mimic one for `alias`
		} else if (!util::at_node<ast::QualifiedBinding>(s)) {
			s.emplace_back(nullptr);
		}

		s.emplace_back(std::move(bind));
		// stack: (qualbind | {}) bind
	} END;
	RULE(alias) {
		// stack: (qualbind | {}) bind? array? bind array??
		auto narr = POP(Array);
		auto nbind = POP(BasicBinding);
		auto arr = POP(Array);
		auto bind = POP(BasicBinding);
		auto mod = POP(QualifiedBinding);

		// If 'alieps' pushed a nullptr on the stack
		if (!mod) { s.pop_back(); }

		PUSH(Rebind, std::move(mod), std::move(bind), std::move(arr), std::move(nbind), std::move(narr));
		// stack: Rebind
	} END;
	RULE(imps) {
		PUSH(QualifiedBinding, POP(BasicBinding));
	} END;
	RULE(imp_alias) {
		// stack: (qualbind basicbind?) | rebind
		if (!util::at_node<ast::Rebind>(s)) {
			auto typ = POP(BasicBinding);
			auto mod = POP(QualifiedBinding);

			if (!typ && mod->elems.size()) {
				// If 'bind' is a var, then mod_path/imps gobbles it into the qualified binding
				// This restores the division to match our expected/desired behavior
				auto bind = std::move(mod->elems.back());

				mod->elems.pop_back();
				if (mod->elems.size() == 0) {
					mod = nullptr;
				}

				PUSH(SingleImport, std::move(mod), std::move(bind));
			} else {
				PUSH(SingleImport, std::move(mod), std::move(typ));
			}
		}
		// stack: singleimport | rebind
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
	RULE(in_assign) {
		// stack: vis pat gen? type? val expr
		auto context = POP(ValExpr);
		action<grammar::asgn_val>::apply(in, s, state);
		PUSH(InAssign, POP(VarAssign), std::move(context));
		// stack: InAssign
	} END;
	INHERIT(asgn_in, in_assign);
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
		auto oper = op->name;

		if (!rhs) {
			rhs = std::make_unique<ast::Future>(true, op->loc);
		}
		
		PUSH(BinOpCall, POP(ValExpr), std::move(rhs), oper);
		// stack: binexpr
	} END;
	INHERIT(binary_cont2, binary_cont1);
	INHERIT(binary_cont3, binary_cont1);
	INHERIT(binary_cont4, binary_cont1);
	INHERIT(binary_cont5, binary_cont1);
	INHERIT(binary_cont6, binary_cont1);
	INHERIT(binary_cont7, binary_cont1);


	// Organizational Tagging
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
	RULE(statement) {
		// stack: annotation* stmt
		auto stmt = POP(Statement);
		auto anots = util::popSeq<ast::LocalAnnotation>(s);

		stmt->annots = std::move(anots);
		s.emplace_back(std::move(stmt));
		// stack: stmt
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