#pragma once
#pragma warning(disable : 4503)

#include "pegtl.hpp"

namespace spero::parser::grammar {
	using namespace tao::pegtl;
#define pstr(x) TAOCPP_PEGTL_STRING((x))
#define key(x) seq<pstr((x)), not_at<ascii::identifier_other>, ig_s> {}


	/*
	* Custom rules
	*/
	using eps = success;
	template<char ch>
	struct spec_char : seq<one<ch>, star<ig>> {};


	/*
	 * Forward declarations
	 */
	struct mvdexpr;		struct mvexpr;
	struct tuple;		struct statement;
	struct index_cont;	struct scope;
	struct _array;		struct assign_pat;
	struct lit;


	/*
	 * Ignored and Special Characters
	 */
	struct one_line_comment : seq<one<'#'>, until<eolf>> {};
	struct multiline_comment : seq<two<'#'>, until<two<'#'>>> {};
	struct whitespace : plus<space> {};
	struct ig : sor<multiline_comment, one_line_comment, whitespace> {};
	struct ig_s : star<ig> {};
	struct obrace : spec_char<'{'> {};
	struct cbrace : spec_char<'}'> {};
	struct obrack : spec_char<'['> {};
	struct cbrack : spec_char<']'> {};
	struct oparen : spec_char<'('> {};
	struct cparen : spec_char<')'> {};
	struct plambda : spec_char<'_'> {};
	struct comma : spec_char<','> {};
	struct endc : spec_char<';'> {};
	struct equals : spec_char<'='> {};
	struct bar : spec_char<'|'> {};
	struct dot : spec_char<'.'> {};


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
	struct klet : key("let");
	struct kdef : key("def");
	struct kstatic : key("static");
	struct kmod : key("mod");
	struct kimpl : key("impl");
	struct kuse : key("use");
	struct kas : key("as");
	struct kfalse : key("false");
	struct ktrue : key("true");
	struct kbreak : key("break");
	struct kcontinue : key("continue");
	struct kret : key("return");
	struct kyield : key("yield");
	struct kwait : key("wait");
	struct vcontext : sor<klet, kdef, kstatic> {};
	struct jump_key : sor<kbreak, kcontinue, kret, kyield, kwait> {};


	/*
	 * Identifiers
	 */
	struct typ_ch : range<'A', 'Z'> {};
	struct typ : seq<typ_ch, ascii::identifier> {};
	struct var : seq<range<'a', 'z'>, ascii::identifier> {};
	struct mod_path : star<one<':'>, not_at<sor<typ_ch, one<':'>>>, var> {};
	struct _mod_path : star<var, one<':'>> {};
	struct varname : seq<var, mod_path> {};
	struct typname : seq<varname, one<':'>, typ> {};
	struct binding : seq<sor<var, typ>, ig_s> {};
	struct unop : eps {};
	struct binop : eps {};
	struct op : sor<unop, binop> {};
	struct pat_tuple : seq<oparen, opt<pattern, star<comma, pattern>>, cparen> {};
	struct pat_adt : seq<typname, ig_s, opt<pat_tuple>> {};
	struct capture_desc : seq<opt<kmut>, opt<one<'&'>, ig_s>> {};
	struct capture : seq<capture_desc, sor<pat_tuple, seq<var, ig_s>>> {};
	struct pattern : sor<one<'_'>, lit, pat_adt, capture> {};
	struct assign_tuple : seq<oparen, assign_pat, star<comma, assign_pat>, cparen> {};
	struct assign_pat : seq<sor<var, op, one<'_'>, assign_tuple>, ig_s> {};


	/*
	 * Type System
	 */
	struct typ_ptr : sor<one<'&'>, one<'*'>, two<'.'>> {};
	struct single_type : seq<typname, ig_s, opt<_array>> {};
	struct ref_type : seq<single_type, opt<typ_ptr, ig_s>> {};
	struct tuple_type : seq<oparen, opt<type, star<comma, type>>, cparen> {};
	struct fn_type : seq<pstr("->"), ig_s, type> {};
	struct tuple_fn_type : seq<tuple_type, opt<fn_type>> {};
	struct mut_type : seq<opt<kmut>, sor<tuple_fn_type, ref_type>> {};
	struct ntype : sor<mut_type, seq<obrace, type, cbrace>> {};
	struct and_type : seq<ntype, star<one<'&'>, ig_s, ntype>> {};
	struct or_type : seq<and_type, star<bar, and_type>> {};
	struct type : or_type {};


	/*
	 * Decorators
	 */
	struct annotation : seq<one<'@'>, var, opt<tuple>> {};
	struct ganot : seq<pstr("@!"), var, opt<tuple>> {};
	struct type_inf : seq<pstr("::"), type> {};
	struct variance : sor<one<'+'>, one<'-'>> {};
	struct variadic : seq<pstr(".."), ig_s> {};
	struct relation : seq<sor<pstr("::"), pstr("!:")>, ig_s, type> {};
	struct type_gen : seq<typ, ig*, opt<variance>, ig_s, opt<variadic>, opt<relation>> {};
	struct val_gen : seq<var, ig*, opt<relation>> {};
	struct gen_part : sor<type_gen, val_gen> {};
	struct _generic : seq<obrack, gen_part, star<comma, gen_part>, cbrack> {};
	struct adt : seq<typ, opt<tuple_type>> {};
	struct adt_dec : seq<adt, star<bar, adt>> {};
	struct arg : seq<var, ig_s, opt<type_inf>> {};
	struct arg_tuple : seq<oparen, opt<arg, star<comma, arg>>, cparen> {};
	struct anon_type : seq<pstr("::"), ig_s, scope> {};


	/*
	 * Dot Control
	 */
	struct dotloop : seq<kloop> {};
	struct dotwhile : seq<kwhile, mvexpr> {};
	struct dotfor : seq<kfor, pattern, kin, mvexpr> {};
	struct dotbranch : seq<if_core, star<elsif_case>, opt<else_case>> {};
	struct dotmatch : seq<kmatch, obrace, plus<_case>, cbrace> {};
	struct dot_ctrl : sor<dotloop, dotwhile, dotfor, dotbranch, dotmatch, jump_key> {};


	/*
	 * Atoms
	 */
	struct bin_body : plus<sor<one<'0'>, one<'1'>>> {};
	struct binary : seq<pstr("0b"), bin_body, ig_s> {};
	struct hex_body : plus<xdigit> {};
	struct hex : seq<pstr("0x"), hex_body, ig_s> {};
	struct decimal : seq<plus<digit>, opt<one<'.'>, plus<digit>>> {};
	struct char_body : seq<opt<one<'\\'>>, any> {};
	struct _char : seq<one<'\''>, char_body, one<'\''>, ig_s> {};
	struct str_body : until<at<one<'"'>>, seq<opt<one<'\\'>>, any>> {};
	struct string : seq<one<'"'>, str_body, one<'"'>, ig_s> {};
	struct lit : sor<binary, hex, decimal, _char, string, kfalse, ktrue> {};
	struct scope : seq<obrace, star<statement>, cbrace> {};
	struct tuple : seq<oparen, opt<mvexpr, star<comma, mvexpr>>, cparen> {};
	struct _array : seq<obrack, opt<mvexpr, star<comma, mvexpr>>, cbrack> {};
	struct lambda : seq<pstr("->"), ig_s, mvexpr> {};
	struct dot_fn : seq<dot, sor<dot_ctrl, index_cont>> {};
	struct fn_tuple : sor<seq<tuple, opt<lambda>>, dot_fn> {};
	struct atom : sor<fn_tuple, _array, lit, plambda, scope> {};


	/*
	 * Statements
	 *
	 * TODO: Split up the organization (can I do this to removes some forward declarations?)
	 */
	struct mod_dec : seq<kmod, varname, sor<endc, eolf>, ig_s> {};
	  // TODO: Find a way of specifying "ig_s" so that the 'eolf' enforcement works
	struct impl : seq<kimpl, single_type, opt<kfor, single_type>, scope> {};
	struct mul_imp : seq<obrace, binding, star<comma, binding>, cbrace> {};
	struct alias : seq<opt<_array>, kas, binding, opt<_array>> {};
	struct imp_alias : seq<binding, opt<alias>> {};
	struct mod_alias : seq<kuse, _mod_path, sor<mul_imp, imp_alias>> {};
	struct type_assign : seq<typ, ig_s, opt<_generic>, equals, sor<adt_dec, arg_tuple, eps>, scope> {};
	struct asgn_val : seq<equals, valexpr, opt<kin, mvexpr>> {};
	struct _interface : seq<type_inf, opt<asgn_val>> {};
	struct var_assign : seq<assign_pat, opt<_generic>, sor<_interface, asgn_val>> {};
	struct assign : seq<vcontext, sor<type_assign, var_assign>> {};
	struct in_assign
		: seq<vcontext, assign_pat, opt<_generic>, opt<type_inf>, equals, valexpr, kin, mvexpr> {};
	struct type_const : seq<typ, ig_s, opt<actcall>, opt<anon_type>> {};
	struct named : seq<varname, opt<one<':'>, type_const>> {};
	struct valcall : sor<atom, seq<op, ig_s>> {};
	struct actcall : sor<seq<_array, opt<tuple>>, tuple> {};
	struct fncall : seq<sor<named, valcall>, star<actcall>> {};
	struct index_cont : seq<fncall, star<dot, fncall>> {};
	struct index : seq<fncall, dot, sor<dot_ctrl, index_cont>> {};
	struct unexpr : seq<opt<unop>, index> {};
	struct binexpr : seq<unexpr, opt<binop, ig_s, opt<unexpr>>> {};


	/*
	 * Control Structures
	 */
	struct forl : seq<kfor, pattern, kin, mvexpr, mvdexpr> {};
	struct whilel : seq<kwhile, mvexpr, mvdexpr> {};
	struct if_core : seq<kif, mvexpr> {};
	struct elsif_case : seq<kelsif, mvexpr, mvdexpr> {};
	struct else_case : seq<kelse, mvdexpr> {};
	struct branch : seq<if_core, mvdexpr, star<elsif_case>, opt<else_case>> {};
	struct _case : seq<pattern, opt<if_core>, pstr("=>"), ig*, mvexpr, opt<endc>> {};
	struct match : seq<kmatch, mvexpr, obrace, plus<_case>, cbrace> {};
	struct jump : seq<jump_key, opt<mvexpr>> {};
	struct loop : seq<kloop, mvdexpr> {};
	struct control : sor<match, forl, whilel, branch, jump, loop> {};
	

	/*
	 * Organizational Structures
	 */
	struct valexpr : seq<opt<kmut>, sor<control, binexpr>, opt<type_inf>> {};
	struct mvexpr : sor<valexpr, in_assign> {};
	struct mvdexpr : seq<opt<kdo>, mvexpr> {};
	struct expr : sor<ganot, mod_alias, assign, seq<opt<kdo>, valexpr>> {};
	struct statement : seq<star<annotation>, expr, opt<endc>> {};
	struct global_statement : sor<mod_dec, impl, statement> {};
	struct program : star<ig_s, global_statement> {};
}