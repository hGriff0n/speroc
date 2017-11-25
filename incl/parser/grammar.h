#pragma once
#pragma warning(disable : 4503)

#include "pegtl.hpp"

namespace spero::parser::grammar {
	using namespace tao::pegtl;


	/*
	 * Macro Defines
	 */
	#define pstr(x) TAOCPP_PEGTL_STRING((x))
	#define key(x) seq<pstr((x)), not_at<ascii::identifier_other>, ig_s> {}
	#define DEFINE_BINPREC_LEVEL(lvl, prev) \
		struct binary_op##lvl : seq<binary_opch##lvl, star<binop_ch>> {}; \
		struct binary_cont##lvl : seq<binary_op##lvl, ig_s, opt<binary_prec##prev>> {}; \
		struct binary_prec##lvl : seq<binary_prec##prev, star<binary_cont##lvl>> {}
	#define DEFINE_BINPREC_LEVEL_OP(lvl, prev, ...) \
		struct binary_opch##lvl : one_of<__VA_ARGS__> {}; \
		DEFINE_BINPREC_LEVEL(lvl, prev)
	#define SENTINEL(name) struct name : seq<eps> {}


	/*
	 * Forward declarations
	 */
	struct mvdexpr;		struct mvexpr;		struct _array;
	struct tuple;		struct statement;	struct assign_pat;
	struct index_cont;	struct scope;		struct pat_lit;
	struct pattern;		struct type;		struct valexpr;


	/*
	 * Custom rules
	 */
	using eps = success;
	template<char... chs>
	struct one_of : sor<one<chs>...> {};
	template<class Rule, char... chs>
	struct _sor : sor<one<chs>..., Rule> {};
	template<class Odelim, class Rule, class Cdelim, class Sep = comma>
	struct opt_sequence : seq<Odelim, opt<Rule, star<Sep, Rule>>, Cdelim> {};
	template<class Odelim, class Rule, class Cdelim, class Sep = comma>
	struct sequence : seq<Odelim, Rule, star<Sep, Rule>, Cdelim> {};


	/*
	 * Ignored Characters
	 */
	struct one_line_comment : seq<one<'#'>, until<eolf>> {};
	struct multiline_comment : seq<two<'#'>, until<two<'#'>>> {};
	struct whitespace : plus<space> {};
	struct ig : sor<multiline_comment, one_line_comment, space> {};
	struct ig_s : star<ig> {};
	struct ign_s : star<not_at<eolf>, ig> {};


	/*
	 * Punctuation
	 *
	 * TODO: Make 'sentinel' variants where necessary
	 */
	template<char ch>
	struct punct : seq<one<ch>, ig_s> {};
	struct obrace : punct<'{'> {};
	struct cbrace : punct<'}'> {};
	struct obrack : punct<'['> {};
	struct cbrack : punct<']'> {};
	struct oparen : punct<'('> {};
	struct cparen : punct<')'> {};
	struct plambda : punct<'_'> {};
	struct comma : punct<','> {};
	struct endc : punct<';'> {};
	struct equals : punct<'='> {};
	struct bar : punct<'|'> {};
	struct and : punct<'+'> {};
	struct dot : punct<'.'> {};
	struct ochar : one<'\''> {};
	struct oquote : one<'"'> {};

	struct errorbrace : ig_s {};
	struct errorparen : ig_s {};
	struct errorbrack : ig_s {};
	struct errorequal : ig_s {};
	struct errorchar : ig_s {};
	struct errorquote : ig_s {};


	/*
	 * Keywords
	 */
	struct kmut : key("mut");
	struct kdo : key("do");
	struct kloop : key("loop");
	struct kmatch : key("match");
	struct kif : key("if");
	struct kelsif : key("elsif");
	struct kelse : key("else");
	struct kfor : key("for");
	struct kin : key("in");
	struct kwhile : key("while");
	struct kmod : key("mod");
	struct kimpl : key("impl");
	struct kuse : key("use");
	struct kas : key("as");
	struct kfalse : key("false");
	struct ktrue : key("true");
	struct klet : key("let");
	struct kdef : key("def");
	struct kpriv : key("private");
	struct kbreak : key("break");
	struct kcontinue : key("continue");
	struct kret : key("return");
	struct kyield : key("yield");
	struct kwait : key("wait");
	struct vcontext : sor<klet, kdef> {};
	struct jump_key : sor<kbreak, kcontinue, kret, kyield, kwait> {};
	struct keywords : sor<jump_key, vcontext, ktrue, kfalse, kas, kuse, kimpl, kmod,				// This is mostly in-case I want to short-cut var-checking at some point
		kwhile, kin, kfor, kelse, kelsif, kif, kmatch, kloop, kdo, kmut> {};


	/*
	 * Identifiers
	 */
	 // TODO: Error if name matches a keyword (analysis? can this even happen anymore (yes, in some contexts, may be error created only)?)
	struct var : seq<range<'a', 'z'>, star<ascii::identifier>> {};
	struct typ_ch : range<'A', 'Z'> {};
	struct typ : seq<typ_ch, star<ascii::identifier>> {};
	struct unop : one_of<'!', '-', '~'> {};
	struct binop_ch : one_of<'!', '$', '%', '^', '&', '*', '?', '<', '>', '|', '/', '\\', '-', '=', '+', '~'> {};
	struct binop : if_then_else<sor<pstr("->"), pstr("=>")>, failure, seq<opt<one<'!'>>, plus<binop_ch>>> {};
	struct op : disable<sor<binop, unop>> {};
	struct pvar : disable<var> {};
	struct ptyp : disable<typ> {};
	struct pname : sor<pvar, ptyp> {};
	struct path_part : seq<pname, opt<_array>> {};
	struct path : star<path_part, one<':'>> {};
	struct pathed_name : seq<path, disable<path_part>> {};
	struct typname : seq<pathed_name> {};
	struct pat_any : disable<plambda> {};
	struct pat_tuple : opt_sequence<oparen, pattern, sor<cparen, errorparen>> {};										// Immediate error if no closing ')'
	struct capture_desc : sor<seq<one<'&'>, ig_s, opt<kmut>>, kmut, eps> {};
	struct pat_adt : seq<pat_tuple> {};
	SENTINEL(resolve_constants);
	struct pat_name_adt : seq<pathed_name, ig_s, if_then_else<at<one<'('>>, pat_adt, resolve_constants>> {};
	struct pat_missing : until<ig, any> {};
	struct capture : seq<capture_desc, sor<pat_tuple, pat_name_adt, pat_missing>> {};
	struct pattern : seq<sor<pat_any, pat_lit, capture>, ig_s> {};
	struct assign_tuple : sequence<oparen, assign_pat, sor<cparen, errorparen>> {};										// Immediate error if no closing ')'
	struct assign_drop : disable<plambda> {};
	struct assign_name : sor<var, op> {};
	struct assign_pat : seq<sor<assign_name, assign_drop, assign_tuple>, ig_s> {};
	struct assign_typ : seq<typ, ig_s> {};


	/*
	 * Type System
	 */
	SENTINEL(errortype);
	struct typ_view : two<'.'> {};
	struct typ_ref : one<'&'> {};
	struct typ_ptr : one<'*'> {};
	struct ptr_styling : sor<typ_view, typ_ref, typ_ptr> {};
	struct single_type : seq<typname, ig_s> {};
	struct ref_type : seq<single_type, opt<ptr_styling, ig_s>> {};
	struct tuple_type : opt_sequence<oparen, type, sor<cparen, errorparen>> {};											// Immediate error if no closing ')'
	struct fn_type : seq<pstr("->"), ig_s, sor<type, errortype>> {};
	struct tuple_fn_type : seq<tuple_type, opt<fn_type>> {};
	struct mut_type : seq<opt<kmut>, sor<tuple_fn_type, ref_type>> {};
	struct braced_type : seq<obrace, type, sor<cbrace, errorbrace>> {};													// Immediate error if no closing '}'
	struct ntype : sor<mut_type, braced_type> {};
	struct and_cont : seq<and, sor<ntype, errortype>> {};
	struct and_type : seq<ntype, star<and_cont>> {};
	struct or_cont : seq<bar, sor<and_type, errortype>> {};
	struct or_type : seq<and_type, star<or_cont>> {};
	struct type : or_type {};


	/*
	 * Decorators
	 */
	struct annotation : seq<one<'@'>, var, opt<tuple>, ig_s> {};														// NOTE: If `var` is missing, this fails to match anything else (an error itself)
	struct ganot : seq<pstr("@!"), var, opt<tuple>, ig_s> {};
	struct type_inf : seq<pstr("::"), ig_s, sor<type, errortype>> {};
	struct variance : sor<one<'+'>, one<'-'>, eps> {};
	struct variadic : seq<pstr(".."), ig_s> {};
	struct relation : seq<sor<pstr("::"), pstr("!:")>, ig_s> {};
	struct type_gen : seq<typ, ig_s, variance, ig_s, opt<variadic>, opt<relation, sor<type, errortype>>> {};
	struct val_gen : seq<var, ig_s, opt<relation, sor<type, errortype>>> {};
	struct gen_parterror : until<at<sor<one<']'>, one<','>>>> {};
	struct gen_part : sor<type_gen, val_gen, gen_parterror> {};
	struct _generic : sequence<obrack, gen_part, sor<cbrack, errorbrack>> {};											// Immediate error if no closing ']'
	struct adt : seq<typ, if_then_else<at<one<'('>>, sor<tuple_type, errortype>, eps>, ig_s> {};						// NOTE: This produces two errors if a '(' is seen but no ')'
	SENTINEL(error_adt);
	struct adt_dec : seq<adt, star<bar, sor<adt, error_adt>>> {};
	struct arg_sentinel : seq<ig_s> {};
	struct arg_inf : seq<pstr("::"), arg_sentinel, sor<type, errortype>> {};
	struct arg : seq<var, ig_s, opt<arg_inf>> {};
	struct arg_tuple : opt_sequence<oparen, arg, sor<cparen, errorparen>> {};											// Immediate error if no closing ')'
	struct anon_type : seq<pstr("::"), ig_s, scope> {};


	/*
	 * Control Structures
	 */
	SENTINEL(missing_gen);
	SENTINEL(missing_fbody);
	struct infor : seq<kin, sor<mvexpr, missing_gen>, sor<mvdexpr, missing_fbody>> {};
	SENTINEL(missing_in);
	SENTINEL(missing_pat);
	// TODO: Look into allowing "for k, v in map ..." syntax
	struct forl : seq<kfor, if_then_else<pattern, sor<infor, missing_in>, missing_pat>> {};								// Immediate error if 'in' not used
	SENTINEL(missing_wtest);
	SENTINEL(missing_wbody);
	struct whilel : seq<kwhile, sor<mvexpr, missing_wtest>, sor<mvdexpr, missing_wbody>> {};
	SENTINEL(missing_lbody);
	struct loop : seq<kloop, sor<mvdexpr, missing_lbody>> {};
	SENTINEL(missing_itest);
	struct if_core : seq<kif, sor<mvexpr, missing_itest>> {};
	SENTINEL(missing_ibody);
	struct if_branch : seq<if_core, sor<mvdexpr, missing_ibody>> {};
	SENTINEL(missing_eitest);
	SENTINEL(missing_eibody);
	struct elseif_key : key("elseif");
	struct elsif_case : seq<sor<kelsif, elseif_key>, sor<mvexpr, missing_eitest>, sor<mvdexpr, missing_eibody>> {};
	SENTINEL(missing_ebody);
	struct else_case : seq<kelse, sor<mvdexpr, missing_ebody>> {};
	struct branch : seq<if_branch, star<elsif_case>, opt<else_case>> {};
	SENTINEL(missing_cpat);
	// TODO: Errors produced with "=> 3" case aren't too informative (keeps saying "no '=>'")
	struct case_pat : seq<pattern, star<comma, sor<pattern, missing_cpat>>> {};
	// TODO: Error if no valid end character (';}\n')
	SENTINEL(missing_arrow);
	SENTINEL(missing_cbody);
	struct _case : seq<case_pat, opt<if_core>, sor<pstr("=>"), missing_arrow>, ig_s, sor<mvexpr, missing_cbody>, opt<endc>> {};
	SENTINEL(missing_mexpr);
	SENTINEL(missing_brace);
	struct cases : seq<sor<obrace, missing_brace>, star<not_at<one<'}'>>, _case>, sor<cbrace, errorbrace>> {};
	struct matchs : seq<kmatch, if_then_else<mvexpr, cases, missing_mexpr>> {};											// Immediate error if no closing '}', no case statements
	struct jump : seq<jump_key, opt<mvexpr>> {};
	struct control : sor<matchs, forl, whilel, branch, jump, loop> {};
	struct dotloop : seq<kloop> {};
	struct dotwhile : seq<kwhile, sor<mvexpr, missing_wtest>> {};
	struct dot_infor : seq<kin, sor<mvexpr, missing_gen>> {};
	SENTINEL(missing_dotpat);
	struct dotfor : seq<kfor, if_then_else<pattern, sor<dot_infor, missing_in>, missing_dotpat>> {};					// Errors: see 'forl'
	struct dotif : seq<if_core> {};
	struct dotbranch : seq<dotif, star<elsif_case>, opt<else_case>> {};
	struct dotmatch : seq<kmatch, sor<obrace, missing_brace>, star<_case>, sor<cbrace, errorbrace>> {};					// Errors: see 'matchs'
	struct dotjump : seq<jump_key> {};
	struct dot_ctrl : sor<dotloop, dotwhile, dotfor, dotbranch, dotmatch, dotjump> {};


	/*
	 * Atoms
	 */
	struct bin_body : star<one_of<'0', '1'>> {};
	struct binary : seq<pstr("0b"), bin_body, ig_s> {};																	// Immediate error if binary body isn't present
	struct hex_body : star<xdigit> {};
	struct hex : seq<pstr("0x"), hex_body, ig_s> {};																	// Immediate error if hex body isn't present
	struct decimal : seq<plus<digit>, opt<one<'.'>, plus<digit>>, ig_s> {};												// NOTE: '.'<eps> needs to be "allowed" for indexing
	struct char_body : seq<any> {};
	struct _char : seq<ochar, opt<one<'\\'>>, char_body, sor<one<'\''>, errorchar>, ig_s> {};							// Immediate error if no closing '''
	struct str_body : until<at<sor<one<'"'>, tao::pegtl::eof>>, seq<opt<one<'\\'>>, any>> {};
	struct string : seq<oquote, str_body, sor<one<'"'>, errorquote>, ig_s> {};											// Immediate error if no closing '"'
	struct lit : sor<binary, hex, decimal, _char, string, kfalse, ktrue> {};
	struct pat_lit : seq<lit> {};
	struct scope : seq<obrace, star<statement>, sor<cbrace, errorbrace>> {};											// Immediate error if no closing '}'
	// TODO: Error if ',' and no mvexpr
	struct tuple : opt_sequence<oparen, mvexpr, sor<cparen, errorparen>> {};											// Immediate error if no closing ')'
	// TODO: Error if ',' and no mvexpr
	struct _array : opt_sequence<obrack, mvexpr, sor<cbrack, errorbrack>> {};											// Immediate error if no closing ']'
	// TODO: Error if no mvexpr used
	struct lambda : seq<pstr("->"), ig_s, mvexpr> {};
	struct fwd_dot : seq<dot> {};
	// TODO: Error if no dot_ctrl or index_cont
	struct dot_fn : seq<fwd_dot, sor<dot_ctrl, index_cont>> {};
	struct fn_tuple : sor<seq<tuple, opt<lambda>>, dot_fn> {};
	struct atom : sor<fn_tuple, _array, lit, plambda, scope> {};


	/*
	 * Expressions
	 */
	// TODO: Error if '=' not used, 'in' not used
	// TODO: Error if wrong keyword used
	// TODO: Error if exprs not used
	struct in_assign : seq<vcontext, assign_pat, opt<_generic>, opt<type_inf>, equals, valexpr, kin, mvexpr> {};
	struct call : enable<tuple> {};
	struct type_const_tail : opt<disable<call>, opt<anon_type>> {};
	struct op_var : seq<binop> {};
	struct pathed_var : sor<pathed_name, op_var> {};
	struct var_val : seq<pathed_var, type_const_tail, ig_s> {};
	struct fncall : seq<sor<atom, var_val>, star<call>> {};
	SENTINEL(indexeps);
	// TODO: Error if '.' and no fncall
	struct index_cont : seq<indexeps, fncall, star<dot, fncall>> {};
	// TODO: Error if '.' and no dot_ctrl or index_cont
	struct index : seq<fncall, opt<dot, sor<dot_ctrl, index_cont>>> {};
	// TODO: Error if unop and no index
	struct unopcall : seq<unop, index> {};
	struct unexpr : sor<unopcall, index> {};


	/*
	 * Statements
	 */
	// TODO: Error if eolf or ';' not used
	struct mod_dec : seq<kmod, pathed_name, ig_s, must<sor<endc, eolf>>, ig_s> {};
	// TODO: Error if no type used
	struct for_type : seq<kfor, single_type> {};
	struct impl_errchars : plus<not_at<disable<obrace>>, any> {};														// Immediate error if unexpected characters encountered
	// TODO: Error if no scope, no type used
	struct impl : seq<kimpl, single_type, opt<for_type>, opt<impl_errchars>, scope> {};
	struct mul_imp : sequence<obrace, seq<ig_s, pname, ig_s>, sor<cbrace, errorbrace>> {};
	struct at_rebind_point : seq<ig_s, sor<disable<_array>, at<kas>>> {};
	SENTINEL(err_rebind);
	struct rebind : seq<kas, path_part> {};
	struct maybe_rebind : sor<rebind, err_rebind> {};
	SENTINEL(import_single);
	struct imp_alias : seq<disable<pname>, if_then_else<at_rebind_point, maybe_rebind, import_single>> {};
	struct mod_alias : seq<kuse, opt<path>, sor<mul_imp, imp_alias>> {};
	// TODO: Error if '=' not used
	// TODO: Error if scope not used
	struct type_assign : seq<assign_typ, opt<_generic>, equals, opt<kmut>, sor<adt_dec, arg_tuple, eps>, scope> {};
	// TODO: Error if no mvexpr
	struct asgn_in : seq<kin, mvexpr> {};
	// TODO: Error if '=' not used
	// TODO: Error if no valexpr
	struct asgn_val : seq<equals, valexpr, opt<asgn_in>> {};
	struct _interface : seq<type_inf, opt<asgn_val>> {};
	// TODO: Error if _interface or asgn_val not used
	struct var_assign : seq<assign_pat, opt<_generic>, sor<_interface, asgn_val>> {};
	// TODO: Error if invalid keyword
	struct assign : seq<vcontext, sor<type_assign, var_assign>> {};


	/*
	 * Binary Precedence
	 *  NOTE: Attach the rules on the "binary_op{n}" structs, not "binary_prec_op{n}"
	 */
	struct binary_prec0 : seq<unexpr> {};
	DEFINE_BINPREC_LEVEL_OP(1, 0, '&', '$', '?', '\\');
	DEFINE_BINPREC_LEVEL_OP(2, 1, '/', '%', '*');
	struct binary_opch3 : sor<one<'+'>, if_then_else<pstr("->"), failure, one<'-'>>> {};
	DEFINE_BINPREC_LEVEL(3, 2);
	struct binary_opch4 : sor<seq<one<'!'>, binop_ch>, if_then_else<pstr("=>"), failure, one<'='>>> {};
	DEFINE_BINPREC_LEVEL(4, 3);
	DEFINE_BINPREC_LEVEL_OP(5, 4, '<', '>');
	DEFINE_BINPREC_LEVEL_OP(6, 5, '^');
	DEFINE_BINPREC_LEVEL_OP(7, 6, '|');
	struct binexpr : seq<binary_prec7> {};
	

	/*
	 * Organizational Structures
	 */
	// TODO: Error if no control or binexpr
	struct valexpr : seq<opt<kmut>, sor<control, binexpr>, opt<type_inf>> {};
	// TODO: Error if unexpected keyword
	// TODO: Error if no valexpr or in_assign
	struct mvexpr : sor<valexpr, in_assign> {};
	struct mvdexpr : seq<opt<kdo>, mvexpr> {};
	struct statement : sor<ganot, mod_dec, mod_alias, impl, assign, seq<opt<kdo>, valexpr>> {};
	// TODO: Error if no statement
	struct annotated : seq<star<annotation>, statement, opt<endc>> {};
	// TODO: Change to using "forward_set" gobbling (try to get as many "correct stuff" as possible, ie. "(3 4) 5")
	struct leftovers : star<any> {};
	// TODO: Look at adding a "catch" rule, for when no expressions match
	struct program : seq<star<ig_s, annotated>, sor<eolf, leftovers>> {};
}

#undef key
#undef DEFINE_BINPREC_LEVEL
#undef DEFINE_BINPREC_LEVE_OP
#undef SENTINEL