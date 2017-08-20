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
	template<class Odelim, class Rule, class Cdelim, class Sep=comma>
	struct opt_sequence : seq<Odelim, opt<Rule, star<Sep, Rule>>, Cdelim> {};
	template<class Odelim, class Rule, class Cdelim, class Sep=comma>
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
	struct punct   : seq<one<ch>, ig_s> {};
	struct obrace  : punct<'{'> {};
	struct cbrace  : punct<'}'> {};
	struct obrack  : punct<'['> {};
	struct cbrack  : punct<']'> {};
	struct oparen  : punct<'('> {};
	struct cparen  : punct<')'> {};
	struct plambda : punct<'_'> {};
	struct comma   : punct<','> {};
	struct endc    : punct<';'> {};
	struct equals  : punct<'='> {};
	struct bar     : punct<'|'> {};
	struct and     : punct<'+'> {};
	struct dot     : punct<'.'> {};


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
	struct kstatic : key("static");
	struct kbreak : key("break");
	struct kcontinue : key("continue");
	struct kret : key("return");
	struct kyield : key("yield");
	struct kwait : key("wait");
	struct vcontext : sor<klet, kdef, kstatic> {};
	struct jump_key : sor<kbreak, kcontinue, kret, kyield, kwait> {};
	// This is mostly in-case I want to short-cut var-checking at some point
	struct keywords : sor<jump_key, vcontext, ktrue, kfalse, kas, kuse, kimpl, kmod,
						kwhile, kin, kfor, kelse, kelsif, kif, kmatch, kloop, kdo, kmut> {};


	/*
	 * Identifiers
	 */
	struct typ_ch : range<'A', 'Z'> {};
	struct typ : seq<typ_ch, star<ascii::identifier>> {};
	struct qualtyp : seq<typ> {};
	struct var : seq<range<'a', 'z'>, star<ascii::identifier>> {};
	struct mod_path : plus<one<':'>, not_at<sor<typ_ch, one<':'>>>, var> {};
	struct varname : seq<var, opt<mod_path>> {};
	struct typname : seq<opt<varname, one<':'>>, typ> {};
	struct binding : seq<sor<var, typ>, ig_s> {};
	struct unop : one_of<'!', '-', '~'> {};
	struct binop_ch : one_of<'!', '$', '%', '^', '&', '*', '?', '<', '>', '|', '/', '\\', '-', '=', '+', '~'> {};
	struct binop : seq<opt<sor<pstr("->"), one<'!'>, pstr("=>")>>, plus<binop_ch>> {};
	struct op : disable<sor<binop, unop>> {};
	struct pat_tuple : opt_sequence<oparen, pattern, cparen> {};
	struct pat_adt : seq<not_at<disable<kmut>>, typname, ig_s, opt<pat_tuple>> {};
	struct capture_desc : sor<seq<one<'&'>, ig_s, opt<kmut>>, kmut> {};
	struct pat_name : seq<var, not_at<one<':'>>, ig_s> {};
	struct desc_eps : eps {};
	struct capture : seq<sor<capture_desc, desc_eps>, sor<pat_tuple, pat_name>> {};
	struct pat_any : disable<plambda> {};
	struct pattern : seq<sor<pat_any, pat_lit, pat_adt, capture>, ig_s> {};
	struct assign_tuple : sequence<oparen, assign_pat, cparen> {};
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
	struct tuple_type : opt_sequence<oparen, type, cparen> {};
	struct fn_type : seq<pstr("->"), ig_s, type> {};
	struct tuple_fn_type : seq<tuple_type, opt<fn_type>> {};
	struct mut_type : seq<opt<kmut>, sor<tuple_fn_type, ref_type>> {};
	struct ntype : sor<mut_type, seq<obrace, type, cbrace>> {};
	struct and_cont : seq<and, ntype> {};
	struct and_type : seq<ntype, star<and_cont>> {};
	struct or_cont : seq<bar, and_type> {};
	struct or_type : seq<and_type, star<or_cont>> {};
	struct type : or_type {};


	/*
	 * Decorators
	 */
	struct annotation : seq<one<'@'>, var, opt<tuple>, ig_s> {};
	struct ganot : seq<pstr("@!"), var, opt<tuple>, ig_s> {};
	struct type_inf : seq<pstr("::"), ig_s, type> {};
	struct veps : seq<eps> {};
	struct variance : one_of<'+', '-'> {};
	struct variadic : seq<pstr(".."), ig_s> {};
	struct relation : seq<sor<pstr("::"), pstr("!:")>, ig_s> {};
	struct type_gen : seq<typ, ig_s, sor<variance, veps>, ig_s, opt<variadic>, opt<relation, type>> {};
	struct val_gen : seq<var, ig_s, opt<relation, type>> {};
	struct gen_part : sor<type_gen, val_gen> {};
	struct _generic : sequence<obrack, gen_part, cbrack> {};
	struct adt : seq<typ, opt<tuple_type>, ig_s> {};
	struct adt_dec : seq<adt, star<bar, adt>> {};
	struct argeps : seq<eps> {};
	struct arg_inf : seq<pstr("::"), argeps, ig_s, type> {};
	struct arg : seq<var, ig_s, opt<arg_inf>> {};
	struct arg_tuple : opt_sequence<oparen, arg, cparen> {};
	struct anon_type : seq<pstr("::"), ig_s, scope> {};


	/*
	 * Control Structures
	 */
	struct forl : seq<kfor, pattern, kin, mvexpr, mvdexpr> {};
	struct whilel : seq<kwhile, mvexpr, mvdexpr> {};
	struct loop : seq<kloop, mvdexpr> {};
	struct if_core : seq<kif, mvexpr> {};
	struct if_branch : seq<if_core, mvdexpr> {};
	struct elsif_case : seq<kelsif, mvexpr, mvdexpr> {};
	struct else_case : seq<kelse, mvdexpr> {};
	struct branch : seq<if_branch, star<elsif_case>, opt<else_case>> {};
	struct case_pat : seq<pattern, star<comma, pattern>> {};
	struct _case : seq<case_pat, opt<if_core>, pstr("=>"), ig_s, mvexpr, opt<endc>> {};
	struct matchs : seq<kmatch, mvexpr, obrace, plus<_case>, cbrace> {};
	struct jump : seq<jump_key, opt<mvexpr>> {};
	struct control : sor<matchs, forl, whilel, branch, jump, loop> {};
	struct dotloop : seq<kloop> {};
	struct dotwhile : seq<kwhile, mvexpr> {};
	struct dotfor : seq<kfor, pattern, kin, mvexpr> {};
	struct dotif : seq<if_core> {};
	struct dotbranch : seq<dotif, star<elsif_case>, opt<else_case>> {};
	struct dotmatch : seq<kmatch, obrace, plus<_case>, cbrace> {};
	struct dotjump : seq<jump_key> {};
	struct dot_ctrl : sor<dotloop, dotwhile, dotfor, dotbranch, dotmatch> {};


	/*
	 * Atoms
	 */
	struct bin_body : plus<one_of<'0', '1'>> {};
	// TODO: Change to my custom error handling
	struct binary : seq<pstr("0b"), must<bin_body>, ig_s> {};
	struct hex_body : plus<xdigit> {};
	// TODO: Change to my custom error handling
	struct hex : seq<pstr("0x"), must<hex_body>, ig_s> {};
	struct decimal : seq<plus<digit>, opt<one<'.'>, plus<digit>>, ig_s> {};
	struct char_body : seq<any> {};
	struct _char : seq<one<'\''>, opt<one<'\\'>>, char_body, one<'\''>, ig_s> {};
	struct str_body : until<at<one<'"'>>, seq<opt<one<'\\'>>, any>> {};
	struct string : seq<one<'"'>, str_body, one<'"'>, ig_s> {};
	struct lit : sor<binary, hex, decimal, _char, string, kfalse, ktrue> {};
	struct pat_lit : seq<lit> {};
	struct scope : seq<obrace, star<statement>, cbrace> {};
	struct tuple : opt_sequence<oparen, mvexpr, cparen> {};
	struct _array : opt_sequence<obrack, mvexpr, cbrack> {};
	struct lambda : seq<pstr("->"), ig_s, mvexpr> {};
	struct dot_eps : eps {};
	struct dot_fn : seq<dot, dot_eps, sor<dot_ctrl, index_cont>> {};
	struct fn_tuple : sor<seq<tuple, opt<lambda>>, dot_fn> {};
	struct atom : sor<fn_tuple, _array, lit, plambda, scope> {};


	/*
	 * Expressions
	 */
	struct in_assign : seq<vcontext, assign_pat, opt<_generic>, opt<type_inf>, equals, valexpr, kin, mvexpr> {};
	struct type_var : seq<opt<one<':'>>, typ, ig_s> {};
	struct op_var : sor<seq<binop, ig_s>, varname> {};
	struct actcall : enable<sor<seq<_array, opt<tuple>>, tuple>> {};
	struct type_const : seq<type_var, opt<disable<actcall>, opt<anon_type>>> {};
	struct var_val : sor<seq<op_var, opt<type_const>>, type_const> {};
	struct fncall : seq<sor<atom, var_val>, star<actcall>> {};
	struct indexeps : seq<eps> {};
	struct index_cont : seq<indexeps, fncall, star<dot, fncall>> {};
	struct index : seq<fncall, opt<dot, sor<dot_ctrl, index_cont>>> {};
	struct unopcall : seq<unop, index> {};
	struct unexpr : sor<unopcall, index> {};


	/*
	 * Statements
	 */
	struct mod_dec : seq<kmod, varname, ign_s, must<sor<endc, eolf>>, ig_s> {};
	    // TODO: Change to my custom error handling
	struct impleps : seq<eps> {};
	struct impl : seq<kimpl, single_type, opt<kfor, single_type, impleps>, scope> {};
	struct alieps : seq<eps> {};
	struct imps : seq<eps> {};
	struct alias : seq<alieps, opt<_array>, kas, binding, opt<_array>> {};
	struct imp_alias : seq<opt<one<':'>, typ>, ig_s, opt<alias>> {};
	struct mul_imp : seq<opt<one<':'>>, sequence<obrace, binding, cbrace>> {};
	struct mod_alias : seq<kuse, opt<var, sor<mod_path, imps>>, sor<mul_imp, imp_alias>> {};
	struct type_assign : seq<assign_typ, opt<_generic>, equals, opt<kmut>, sor<adt_dec, arg_tuple, eps>, scope> {};
	struct asgn_in : seq<kin, mvexpr> {};
	struct asgn_val : seq<equals, valexpr, opt<asgn_in>> {};
	struct _interface : seq<type_inf, opt<asgn_val>> {};
	struct var_assign : seq<assign_pat, opt<_generic>, sor<_interface, asgn_val>> {};
	struct assign : seq<vcontext, sor<type_assign, var_assign>> {};


	/*
	 * Binary Precedence
	 *  NOTE: Attach the rules on the "binary_op{n}" structs, not "binary_prec_op{n}"
	 */
	struct binary_prec0 : seq<unexpr> {};
	DEFINE_BINPREC_LEVEL_OP(1, 0, '&', '$', '?', '\\');
	DEFINE_BINPREC_LEVEL_OP(2, 1, '/', '%', '*');
	struct binary_opch3 : _sor<seq<pstr("->"), binop_ch>, '+', '-'> {};
	DEFINE_BINPREC_LEVEL(3, 2);
	struct binary_opch4 : sor<seq<one<'!'>, binop_ch>,
		if_then_else<seq<pstr("=>"), not_at<binop_ch>>, failure, one<'='>>> {};
	DEFINE_BINPREC_LEVEL(4, 3);
	DEFINE_BINPREC_LEVEL_OP(5, 4, '<', '>');
	DEFINE_BINPREC_LEVEL_OP(6, 5, '^');
	DEFINE_BINPREC_LEVEL_OP(7, 6, '|');
	struct binexpr : seq<binary_prec7> {};
	

	/*
	 * Organizational Structures
	 */
	struct valexpr : seq<opt<kmut>, sor<control, binexpr>, opt<type_inf>> {};
	struct mvexpr : sor<valexpr, in_assign> {};
	struct mvdexpr : seq<opt<kdo>, mvexpr> {};
	struct expr : sor<ganot, mod_alias, assign, seq<opt<kdo>, valexpr>> {};
	struct statement : seq<star<annotation>, expr, opt<endc>> {};
	struct global_statement : sor<mod_dec, impl, statement> {};
	struct program : seq<star<ig_s, global_statement>, must<eolf>> {};
}

#undef key
#undef DEFINE_BINPREC_LEVEL
#undef DEFINE_BINPREC_LEVE_OP