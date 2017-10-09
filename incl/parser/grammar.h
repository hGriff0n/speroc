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
	struct typ_ch : range<'A', 'Z'> {};
	struct typ : seq<typ_ch, star<ascii::identifier>> {};
	struct qualtyp : seq<typ> {};
	// TODO: Error if name matches a keyword (analysis?)
	struct var : seq<range<'a', 'z'>, star<ascii::identifier>> {};
	// TODO: Error if var not used (deferred)
	struct mod_path : plus<one<':'>, not_at<sor<typ_ch, one<':'>>>, var> {};
	struct varname : seq<var, opt<mod_path>> {};
	// TODO: Error if typ not used (deferred)
	struct type_path : seq<one<':'>, typ> {};
	struct typname : sor<seq<varname, plus<type_path>>, typ> {};
	struct unop : one_of<'!', '-', '~'> {};
	struct binop_ch : one_of<'!', '$', '%', '^', '&', '*', '?', '<', '>', '|', '/', '\\', '-', '=', '+', '~'> {};
	struct binop :
		if_then_else<sor<pstr("->"), pstr("=>")>, failure, seq<opt<one<'!'>>, plus<binop_ch>>> {};
	struct op : disable<sor<binop, unop>> {};
	// TODO: Error if ',' not used (immediate)
	struct pat_tuple : opt_sequence<oparen, pattern, sor<cparen, errorparen>> {};										// Immediate error if no closing ')'
	struct pat_adt : seq<not_at<disable<kmut>>, typname, ig_s, opt<pat_tuple>> {};
	struct capture_desc : sor<seq<one<'&'>, ig_s, opt<kmut>>, kmut, eps> {};
	struct pat_name : seq<var, not_at<one<':'>>, ig_s> {};
	// TODO: Error if invalid capture_desc used, pat_tuple or pat_name not used (deferred)
	struct capture : seq<capture_desc, sor<pat_tuple, pat_name>> {};
	struct pat_any : disable<plambda> {};
	struct pattern : seq<sor<pat_any, pat_lit, pat_adt, capture>, ig_s> {};
	// TODO: Error if ',' not used (immediate)
	struct assign_tuple : sequence<oparen, assign_pat, sor<cparen, errorparen>> {};										// Immediate error if no closing ')'
	struct assign_drop : disable<plambda> {};
	struct assign_name : sor<var, op> {};
	struct assign_pat : seq<sor<assign_name, assign_drop, assign_tuple>, ig_s> {};
	struct assign_typ : seq<typ, ig_s> {};


	/*
	 * Type System
	 */
	struct typ_view : two<'.'> {};
	struct typ_ref : one<'&'> {};
	struct typ_ptr : one<'*'> {};
	struct ptr_styling : sor<typ_view, typ_ref, typ_ptr> {};
	struct single_type : seq<typname, ig_s, opt<_array>> {};
	struct ref_type : seq<single_type, opt<ptr_styling, ig_s>> {};
	// TODO: Error if ',' not used (immediate)
	struct tuple_type : opt_sequence<oparen, type, sor<cparen, errorparen>> {};											// Immediate error if no closing ')'
	// TODO: Error if no type used (deferred)
	struct fn_type : seq<pstr("->"), ig_s, type> {};
	struct tuple_fn_type : seq<tuple_type, opt<fn_type>> {};
	struct mut_type : seq<opt<kmut>, sor<tuple_fn_type, ref_type>> {};
	struct braced_type : seq<obrace, type, sor<cbrace, errorbrace>> {};													// Immediate error if no closing '}'
	struct ntype : sor<mut_type, braced_type> {};
	// TODO: Error if no type used (deferred)
	struct and_cont : seq<and, ntype> {};
	struct and_type : seq<ntype, star<and_cont>> {};
	// TODO: Error if no type used (deferred)
	struct or_cont : seq<bar, and_type> {};
	struct or_type : seq<and_type, star<or_cont>> {};
	struct type : or_type {};


	/*
	 * Decorators
	 */
	// TODO: Error if no var (deferred)
	struct annotation : seq<one<'@'>, var, opt<tuple>, ig_s> {};
	// TODO: Error if no var (deferred)
	struct ganot : seq<pstr("@!"), var, opt<tuple>, ig_s> {};
	// TODO: Error if no type used (deferred)
	struct type_inf : seq<pstr("::"), ig_s, type> {};
	struct variance : sor<one<'+'>, one<'-'>, eps> {};
	struct variadic : seq<pstr(".."), ig_s> {};
	struct relation : seq<sor<pstr("::"), pstr("!:")>, ig_s> {};
	// TODO: Error if relation and no type used (deferred)
	struct type_gen : seq<typ, ig_s, variance, ig_s, opt<variadic>, opt<relation, type>> {};
	// TODO: Error if relation and no type used (deferred)
	struct val_gen : seq<var, ig_s, opt<relation, type>> {};
	// TODO: Error if not type_gen, or val_gen (immediate?)
	struct gen_part : sor<type_gen, val_gen> {};
	struct _generic : sequence<obrack, gen_part, sor<cbrack, errorbrack>> {};											// Immediate error if no closing ']'
	// TODO: Error if 'typ' not used, at '(' and no tuple_type (deferred)
	struct adt : seq<typ, opt<tuple_type>, ig_s> {};
	struct adt_dec : seq<adt, star<bar, adt>> {};
	struct arg_sentinel : seq<ig_s> {};
	// TODO: Error if no type used (deferred)
	struct arg_inf : seq<pstr("::"), arg_sentinel, type> {};
	struct arg : seq<var, ig_s, opt<arg_inf>> {};
	// TODO: Error if no ',' used (immediate)
	struct arg_tuple : opt_sequence<oparen, arg, sor<cparen, errorparen>> {};											// Immediate error if no closing ')'
	struct anon_type : seq<pstr("::"), ig_s, scope> {};


	/*
	 * Control Structures
	 */
	// TODO: Error if no mvexpr or mvdexpr (deferred)
	struct infor : seq<kin, mvexpr, mvdexpr> {};
	struct missing_in : seq<eps> {};
	struct forl : seq<kfor, pattern, sor<infor, missing_in>> {};														// Immediate error if 'in' not used
	// TODO: Error if no mvexpr (deferred)
	struct whilel : seq<kwhile, mvexpr, mvdexpr> {};
	// TODO: Error if no mvexpr (deferred)
	struct loop : seq<kloop, mvdexpr> {};
	// TODO: Error if no mvexpr (deferred)
	struct if_core : seq<kif, mvexpr> {};
	// TODO: Error if no mvexpr (deferred)
	struct if_branch : seq<if_core, mvdexpr> {};
	// TODO: Error if 'elseif' used (immediate)
	// TODO: Error if no mvexpr (deferred)
	struct elsif_case : seq<kelsif, mvexpr, mvdexpr> {};
	// TODO: Error if no mvexpr (deferred)
	struct else_case : seq<kelse, mvdexpr> {};
	struct branch : seq<if_branch, star<elsif_case>, opt<else_case>> {};
	struct case_pat : seq<pattern, star<comma, pattern>> {};
	// TODO: Error if no '=>' used (deferred)
	// TODO: Error if no valid end character (';}\n') (immediate?)
	struct _case : seq<case_pat, opt<if_core>, pstr("=>"), ig_s, mvexpr, opt<endc>> {};
	// TODO: Error if no opening '{' (immediate-deferred)
	struct matchs : seq<kmatch, mvexpr, obrace, star<_case>, sor<cbrace, errorbrace>> {};								// Immediate error if no closing '}', no case statements
	struct jump : seq<jump_key, opt<mvexpr>> {};
	struct control : sor<matchs, forl, whilel, branch, jump, loop> {};
	struct dotloop : seq<kloop> {};
	// TODO: Error if no mvexpr (deferred)
	struct dotwhile : seq<kwhile, mvexpr> {};
	// TODO: Error if no 'mvexpr' (deferred)
	struct dot_infor : seq<kin, mvexpr> {};
	struct dotfor : seq<kfor, pattern, sor<dot_infor, missing_in>> {};													// Errors: see 'forl'
	struct dotif : seq<if_core> {};
	struct dotbranch : seq<dotif, star<elsif_case>, opt<else_case>> {};
	struct dotmatch : seq<kmatch, obrace, star<_case>, sor<cbrace, errorbrace>> {};										// Errors: see 'matchs'
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
	// TODO: Error if ',' and no mvexpr (immediate)
	struct tuple : opt_sequence<oparen, mvexpr, sor<cparen, errorparen>> {};											// Immediate error if no closing ')'
	// TODO: Error if ',' and no mvexpr (immediate)
	struct _array : opt_sequence<obrack, mvexpr, sor<cbrack, errorbrack>> {};											// Immediate error if no closing ']'
	// TODO: Error if no mvexpr used (deferred)
	struct lambda : seq<pstr("->"), ig_s, mvexpr> {};
	struct fwd_dot : seq<dot> {};
	// TODO: Error if no dot_ctrl or index_cont (deferred)
	struct dot_fn : seq<fwd_dot, sor<dot_ctrl, index_cont>> {};
	struct fn_tuple : sor<seq<tuple, opt<lambda>>, dot_fn> {};
	struct atom : sor<fn_tuple, _array, lit, plambda, scope> {};


	/*
	 * Expressions
	 */
	// TODO: Error if '=' not used, 'in' not used (immediate)
	// TODO: Error if wrong keyword used (deferred?)
	// TODO: Error if exprs not used (deferred)
	struct in_assign : seq<vcontext, assign_pat, opt<_generic>, opt<type_inf>, equals, valexpr, kin, mvexpr> {};
	struct type_var : seq<plus<type_path>, ig_s, opt<_array>> {};
	struct op_var : seq<binop> {};
	struct vareps : opt<_array> {};
	struct call : enable<tuple> {};
	struct type_const_tail : seq<opt<_array>, opt<disable<call>, opt<anon_type>>> {};
	struct raw_const : seq<typ, ig_s, type_const_tail> {};
	struct type_const : seq<type_var, ig_s, type_const_tail> {};
	struct var_val : sor<seq<varname, sor<type_const, vareps>>, raw_const, op_var> {};
	struct fncall : seq<sor<atom, var_val>, star<call>> {};
	struct indexeps : seq<eps> {};
	// TODO: Error if '.' and no fncall (deferred)
	struct index_cont : seq<indexeps, fncall, star<dot, fncall>> {};
	// TODO: Error if '.' and no dot_ctrl or index_cont (deferred)
	struct index : seq<fncall, opt<dot, sor<dot_ctrl, index_cont>>> {};
	// TODO: Error if unop and no index (deferred)
	struct unopcall : seq<unop, index> {};
	struct unexpr : sor<unopcall, index> {};


	/*
	 * Statements
	 */
	// TODO: Error if eolf or ';' not used (immediate)
	struct mod_dec : seq<kmod, varname, ign_s, must<sor<endc, eolf>>, ig_s> {};
	// TODO: Error if no type used (deferred)
	struct for_type : seq<kfor, single_type> {};
	struct impl_errchars : plus<not_at<disable<obrace>>, any> {};														// Immediate error if unexpected characters encountered
	// TODO: Error if no scope, no type used (deferred)
	struct impl : seq<kimpl, single_type, opt<for_type>, opt<impl_errchars>, scope> {};

	// TODO: Need to add in error reporting for these definitions
	// Then I need to move to using 'paths' throughout the entire codebase
		// Shrink the number of definitions, move a lot of these outside of the immediate area
	struct pvar : disable<var> {};
	struct ptyp : disable<typ> {};
	struct pname : sor<pvar, ptyp> {};
	struct path_part : seq<pname, opt<_array>> {};
	struct path : star<path_part, one<':'>> {};
	struct pathed_name : seq<path, disable<path_part>> {};

	struct mul_imp : sequence<obrace, seq<ig_s, pname, ig_s>, sor<cbrace, errorbrace>> {};
	struct at_rebind_point : seq<ig_s, sor<disable<_array>, at<kas>>> {};
	struct rebind : seq<kas, path_part> {};
	struct import_single : seq<eps> {};
	// TODO: Error if at rebind point and no rebind
	struct imp_alias : seq<disable<pname>, if_then_else<at_rebind_point, rebind, import_single>> {};
	struct mod_alias : seq<kuse, opt<path>, sor<mul_imp, imp_alias>> {};

	// TODO: Error if '=' not used (immediate)
	// TODO: Error if scope not used (deferred)
	struct type_assign : seq<assign_typ, opt<_generic>, equals, opt<kmut>, sor<adt_dec, arg_tuple, eps>, scope> {};
	// TODO: Error if no mvexpr (deferred)
	struct asgn_in : seq<kin, mvexpr> {};
	// TODO: Error if '=' not used (immediate)
	// TODO: Error if no valexpr (deferred)
	struct asgn_val : seq<equals, valexpr, opt<asgn_in>> {};
	struct _interface : seq<type_inf, opt<asgn_val>> {};
	// TODO: Error if _interface or asgn_val not used (deferred)
	struct var_assign : seq<assign_pat, opt<_generic>, sor<_interface, asgn_val>> {};
	// TODO: Error if invalid keyword (immediate?)
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
	// TODO: Error if no control or binexpr (deferred)
	struct valexpr : seq<opt<kmut>, sor<control, binexpr>, opt<type_inf>> {};
	// TODO: Error if unexpected keyword (immediate)
	// TODO: Error if no valexpr or in_assign (deferred)
	struct mvexpr : sor<valexpr, in_assign> {};
	struct mvdexpr : seq<opt<kdo>, mvexpr> {};
	struct statement : sor<ganot, mod_dec, mod_alias, impl, assign, seq<opt<kdo>, valexpr>> {};
	// TODO: Error if no statement (deferred)
	struct annotated : seq<star<annotation>, statement, opt<endc>> {};
	// TODO: Look at "forward_set" gobbling
	// TODO: Look at adding a "catch" rule, for when no expressions match
	struct program : seq<star<ig_s, annotated>, must<eolf>> {};
}

#undef key
#undef DEFINE_BINPREC_LEVEL
#undef DEFINE_BINPREC_LEVE_OP