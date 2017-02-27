#pragma once
#pragma warning(disable : 4503)

#include "pegtl.hh"

namespace spero::parser::grammar {
	using namespace pegtl;
	#define pstr(x) pegtl_string_t((x))
	#define key(x) seq<pstr((x)), not_at<ascii::identifier_other>, ig_s> {}

	// Forward Declarations
	struct array; struct expr; struct scope; struct atom; struct pattern;
	struct var_pattern; struct valexpr; struct fn_dot_ctrl; struct inf_fn_type;
	struct mut_type; struct con_tuple; struct range;

	// Ignore Characters
	struct one_line_comment : seq<one<'#'>, until<eolf>> {};
	struct multiline_comment : seq<two<'#'>, until<two<'#'>>> {};
	struct whitespace : plus<space> {};
	struct ig : sor<multiline_comment, one_line_comment, whitespace> {};
	struct ig_s : star<ig> {};
	struct eps : success {};
	struct opt_eps : eps {};

	// Special Characters
	template<char ch>
	struct spec_char : seq<one<ch>, star<ig>> {};
	struct obrace : spec_char<'{'> {};
	struct cbrace : spec_char<'}'> {};
	struct obrack : spec_char<'['> {};
	struct cbrack : spec_char<']'> {};
	struct oparen : spec_char<'('> {};
	struct cparen : spec_char<')'> {};
	struct placeholder : spec_char<'_'> {};
	struct id_other : ranges<'a', 'z', 'A', 'Z', '0', '9', '_'> {};

	// Helper Structs
	template<class Rule, char sep = ','>
	struct sequ : list<seq<ig_s, Rule>, one<sep>> {};
	template<class Rule, class Must>
	struct if_then : if_then_else<Rule, Must, failure> {};

	//
	// Language Keywords
	//
	struct k_let : key("let");
	struct k_def : key("def");
	struct k_static : key("static");
	struct k_mut : key("mut");
	struct k_do : key("do");
	struct k_mod : key("mod");
	struct k_use : key("use");
	struct k_match : key("match");
	struct k_if : key("if");
	struct k_elsif : key("elsif");
	struct k_else : key("else");
	struct k_while : key("while");
	struct k_for : key("for");
	struct k_loop : key("loop");
	struct k_break : key("break");
	struct k_continue : key("continue");
	struct k_yield : key("yield");
	struct k_ret : key("return");
	struct k_wait : key("wait");
	struct k_impl : key("impls");
	struct k_in : key("in");
	struct b_false : key("false");
	struct b_true : key("true");
	struct vcontext : sor<k_let, k_def, k_static> {};
	struct jump_keys : sor<k_break, k_continue, k_ret, k_yield> {};
	struct keyword : sor<vcontext, jump_keys, placeholder, k_mut, k_mod, k_use, k_match, k_if, k_elsif, k_else, k_while, k_for, k_loop, k_wait, k_impl, k_in, b_false, b_true> {};

	//
	// Language Bindings
	//
	struct var : seq<not_at<keyword>, ranges<'a', 'z', '_'>, star<id_other>, sor<one<'!'>, one<'?'>, eps>> {};
	struct typ : seq<ascii::range<'A', 'Z'>, star<id_other>> {};
	struct op_characters : sor<one<'!'>, one<'$'>, one<'%'>, one<'^'>, one<'&'>, one<'*'>, one<'?'>, one<'<'>,
		one<'>'>, one<'|'>, one<'`'>, one<'/'>, one<'\\'>, one<'-'>, one<'='>, one<'+'>, one<'~'>> {};
	struct op : seq<sor<one<'&'>, one<'~'>, one<'!'>, one<'?'>, pstr("->"), eps>, plus<op_characters>, ig_s> {};
	struct variable : seq<var, ig_s> {};
	struct name_path_part : if_then<sor<var, typ>, one<':'>> {};
	struct name_eps : seq<eps> {};
	struct name_path : seq<name_eps, star<name_path_part>> {};
	struct typ_pointer : sor<one<'&'>, one<'*'>, two<'.'>, eps> {};
	struct type : seq<name_path, disable<typ>, ig_s, opt<array>, typ_pointer, ig_s> {};
	struct binding : sor<seq<name_path, disable<sor<var, typ>>, ig_s>, op> {};

	//
	// Language Literals
	//
	struct hex_body : plus<xdigit> {};
	struct hex : seq<pstr("0x"), hex_body, ig_s> {};
	struct bin_body : plus<sor<one<'0'>, one<'1'>>> {};
	struct bin : seq<pstr("0b"), bin_body, ig_s> {};
	struct dec : seq<plus<digit>, one<'.'>, plus<digit>> {};
	struct integer : plus<digit> {};
	struct num : seq<sor<dec, integer>, ig_s> {};
	struct str_body : until<at<one<'"'>>, seq<opt<one<'\\'>>, any>> {};
	struct str : seq<one<'"'>, str_body, one<'"'>, ig_s> {};
	struct char_body : seq<opt<one<'\\'>>, any> {};
	//struct character : seq<one<'\''>, if_then_else<char_body, one<'\''>, err_char_no_close>, ig_s> {};
	struct character : seq<one<'\''>, char_body, one<'\''>, ig_s> {};
	//struct tuple : seq<oparen, opt<sequ<valexpr>>, if_then_else<cparen, eps, seq<err_tuple_no_close>>> {};
	struct tuple : seq<oparen, opt<sequ<valexpr>>, cparen> {};
	//struct array : seq<obrack, opt<sequ<valexpr>>, if_then_else<cbrack, eps, seq<err_array_no_close>>> {};
	struct array : seq<obrack, opt<sequ<valexpr>>, cbrack> {};
	struct fn_rettype : if_then<at<mut_type>, seq<mut_type, ig_s, scope>> {};
	struct op_forward : seq<op, opt<valexpr>> {};
	struct fn_forward : seq<one<'.'>, sor<fn_dot_ctrl, op_forward, valexpr>> {};			// Don't need to anything for the valexpr case, will be handled in analysis
	struct fn_def : seq<pstr("->"), ig_s, sor<fn_rettype, fn_forward, valexpr>> {};
	struct fn_or_tuple : sor<fn_forward, seq<tuple, opt<fn_def>>> {};
	struct lit : sor<hex, bin, num, str, character, b_false, b_true, array, fn_or_tuple> {};

	//
	// Language Atoms
	//
	struct anon_sep : seq<pstr(":::")> {};
	struct anon_type : seq<anon_sep, ig_s, opt<con_tuple>, scope> {};
	//struct inst_array : seq<obrack, sequ<sor<type, valexpr>>, cbrack> {};
	struct scope : seq<obrace, star<expr>, cbrace> {};
	struct wait_stmt : seq<k_wait, valexpr> {};
	struct atom : seq<sor<scope, wait_stmt, lit, binding>, ig_s> {};
	struct fnseq : sor<seq<array, opt<anon_type>, opt<tuple>>, seq<anon_type, opt<tuple>>, tuple> {};
	struct fneps : eps {};
	struct fncall : seq<atom, star<fnseq>, fneps> {};

	//
	// Language Decorators
	//
	// anexpr = expr / keyword
	// antuple = oparen (anexpr ("," ig* anexpr)*)? cparen
	struct unary : sor<one<'&'>, one<'!'>, one<'-'>, one<'~'>> {};
	struct annotation : seq<one<'@'>, var, not_at<one<'!'>>, opt<tuple>> {};
	struct global_annotation : seq<one<'@'>, disable<var>, one<'!'>, opt<tuple>> {};
	struct mod_dec : seq<k_mod, list<var, one<':'>>, ig_s> {};
	struct mut_type : seq<opt<k_mut>, type> {};		// mut_type doesn't match type tuples
	struct inf_type : sor<mut_type, inf_fn_type> {};
	struct type_tuple : seq<oparen, sequ<inf_type>, cparen> {};
	struct inf_fn_type : seq<type_tuple, opt<ig_s, pstr("->"), ig_s, inf_type>> {};
	struct inf : seq<pstr("::"), ig_s, inf_type> {};
	struct gen_variance : sor<one<'+'>, one<'-'>, two<'.'>, eps> {};
	struct gen_subrel : sor<pstr("::"), pstr("!:"), one<'>'>, one<'<'>> {};
	struct gen_subtype : seq<gen_subrel, ig_s, type> {};
	struct gen_type : seq<typ, gen_variance, ig_s, opt<gen_subtype>> {};
	struct gen_val : seq<variable, opt<pstr("::"), ig_s, type>, opt<one<'='>, ig_s, expr>> {};
	struct gen_part : sor<gen_type, gen_val> {};
	struct generic : seq<obrack, sequ<gen_part>, cbrack> {};
	struct use_any : seq<placeholder> {};
	struct use_one : seq<var> {};
	struct use_typ : seq<typ> {};
	template<class Gram>
	struct inst_rebind : if_then_else<at<obrack>, seq<array, ig_s, pstr("as"), ig_s, Gram>, opt<pstr("as"), ig_s, Gram>> {};
	struct use_rebind : sor<seq<disable<use_one>, ig_s, inst_rebind<var>>, seq<use_typ, ig_s, inst_rebind<typ>>> {};
	struct alt_rebind : sor<seq<use_one, ig_s, inst_rebind<var>, ig_s>, seq<use_typ, ig_s, inst_rebind<typ>, ig_s>> {};
	struct use_many : seq<obrace, sequ<alt_rebind>, cbrace> {};
	struct use_final : sor<seq<use_any, opt<one<':'>, use_any>>, use_many, use_rebind> {};

	//
	// Patterns
	//
	struct var_tuple : seq<oparen, opt<sequ<var_pattern>>, cparen> {};
	struct var_pattern : sor<var, op, var_tuple> {};
	struct var_type : seq<typ> {};
	struct capture_dec : seq<opt<disable<k_mut>>, opt<one<'&'>>> {};
	struct tuple_pat : seq<oparen, sequ<pattern>, one<')'>> {};
	struct pat_adt : seq<typ, opt<tuple_pat>> {};
	struct pat_tuple : seq<opt<disable<capture_dec>>, ig_s, tuple_pat> {};
	struct pat_var : seq<opt<capture_dec>, ig_s, var> {};
	struct pat_val : sor<hex, bin, num, str, character, b_false, b_true, array> {};
	struct pat_any : seq<placeholder> {};
	struct pattern : sor<pat_any, pat_var, pat_tuple, pat_adt, pat_val> {};

	//
	// Assignment Grammar
	//
	struct adt_con : seq<typ, opt<type_tuple>, ig_s> {};
	struct named_arg : seq<var, ig_s, opt<inf>> {};
	struct con_tuple : seq<oparen, opt<sequ<named_arg>>, cparen> {};
	struct cons : seq<star<adt_con, one<'|'>, ig_s>, sor<con_tuple, disable<adt_con>, eps>> {};
	struct in_binding : seq<k_in, ig_s, valexpr> {};
	struct assign_val : seq<one<'='>, ig_s, valexpr, opt<in_binding>> {};
	struct var_assign : seq<var_pattern, ig_s, opt<generic>, sor<seq<inf, opt<assign_val>>, assign_val>> {};
	struct lhs_inher : seq<inf, one<'='>, ig_s> {};
	//struct rhs_inf : seq<typ, ig_s, two<':'>, ig_s> {};
	struct rhs_inf : if_then<at<typ, ig_s, two<':'>>, seq<typ, ig_s, two<':'>, ig_s>> {};
	struct rhs_inher : seq<one<'='>, ig_s, opt<rhs_inf>> {};
	struct type_assign : seq<var_type, not_at<oparen>, ig_s, opt<generic>, sor<lhs_inher, rhs_inher>, cons, scope> {};
	struct assign : seq<vcontext, sor<type_assign, var_assign>> {};

	//
	// Control Flow Grammar
	//
	struct if_core : seq<k_if, valexpr, opt<k_do>, valexpr> {};
	struct if_dot_core : seq<k_if, valexpr> {};
	struct fn_if_core : seq<k_if, valexpr> {};
	struct elsif_rule : seq<k_elsif, valexpr, opt<k_do>, valexpr> {};
	struct else_rule : seq<k_else, valexpr> {};
	struct elses : seq<star<elsif_rule>, opt<else_rule>> {};
	struct branch : seq<if_core, elses> {};
	struct loop : seq<k_loop, valexpr> {};
	struct while_core : seq<k_while, valexpr> {};
	struct while_l : seq<while_core, opt<k_do>, valexpr> {};
	struct for_core : seq<k_for, pattern, ig_s, k_in, valexpr> {};
	struct for_l : seq<for_core, opt<k_do>, valexpr> {};
	struct jumps : seq<jump_keys, opt<valexpr>> {};
	struct case_if : seq<k_if, valexpr> {};
	struct case_stmt : seq<sequ<seq<pattern, ig_s>>, ig_s, opt<case_if>, pstr("->"), ig_s, valexpr> {};
	struct match_expr : seq<k_match, fncall, obrace, plus<case_stmt>, cbrace> {};
	struct dot_match : seq<k_match, obrace, plus<case_stmt>, cbrace> {};
	struct dot_while : seq<while_core> {};
	struct dot_for : seq<for_core> {};
	struct dot_jmp : seq<jump_keys> {};
	struct dot_loop : seq<k_loop> {};
	struct dot_if : seq<if_dot_core, elses> {};
	struct dot_ctrl : sor<dot_loop, dot_jmp, dot_while, dot_match, dot_for, dot_if> {};
	struct fn_dot_match : seq<k_match, obrace, plus<case_stmt>, cbrace> {};
	struct fn_dot_while : seq<while_core> {};
	struct fn_dot_for : seq<for_core> {};
	struct fn_dot_jmp : seq<jump_keys> {};
	struct fn_dot_loop : seq<k_loop> {};
	struct fn_dot_if : seq<fn_if_core, elses> {};
	struct fn_dot_ctrl : sor<fn_dot_loop, fn_dot_jmp, fn_dot_while, fn_dot_match, fn_dot_for, fn_dot_if> {};

	//
	// Binary Operator Precedence
	//
	struct op_prec_1 : seq<sor<one<'&'>, one<'$'>, one<'?'>, one<'`'>, one<'\\'>>, star<op_characters>> {};
	struct _binary_prec_1 : seq<op_prec_1, ig_s, opt<range>> {};
	struct binary_prec_1 : seq<range, star<_binary_prec_1>> {};
	struct op_prec_2 : seq<sor<one<'/'>, one<'%'>, one<'*'>>, star<op_characters>> {};
	struct _binary_prec_2 : seq<op_prec_2, ig_s, opt<binary_prec_1>> {};
	struct binary_prec_2 : seq<binary_prec_1, star<_binary_prec_2>> {};
	struct op_prec_3 : if_then_else<pstr("->"), plus<op_characters>, seq<sor<one<'+'>, one<'-'>>, star<op_characters>>> {};
	struct _binary_prec_3 : seq<op_prec_3, ig_s, opt<binary_prec_2>> {};
	struct binary_prec_3 : seq<binary_prec_2, star<_binary_prec_3>> {};
	struct op_prec_4 : seq<sor<seq<one<'!'>, plus<op_characters>>, seq<one<'='>, star<op_characters>>>> {};
	struct _binary_prec_4 : seq<op_prec_4, ig_s, opt<binary_prec_3>> {};
	struct binary_prec_4 : seq<binary_prec_3, star<_binary_prec_4>> {};
	struct op_prec_5 : seq<sor<one<'&'>, one<'<'>, one<'>'>>, star<op_characters>> {};
	struct _binary_prec_5 : seq<op_prec_5, ig_s, opt<binary_prec_4>> {};
	struct binary_prec_5 : seq<binary_prec_4, star<_binary_prec_5>> {};
	struct op_prec_6 : seq<one<'^'>, star<op_characters>> {};
	struct _binary_prec_6 : seq<op_prec_6, ig_s, opt<binary_prec_5>> {};
	struct binary_prec_6 : seq<binary_prec_5, star<_binary_prec_6>> {};
	struct op_prec_7 : seq<one<'|'>, star<op_characters>> {};
	struct _binary_prec_7 : seq<op_prec_7, ig_s, opt<binary_prec_6>> {};
	struct binary_prec_7 : seq<binary_prec_6, star<_binary_prec_7>> {};
	struct binary : seq<binary_prec_7> {};

	//
	// Language Expressions
	//
	struct _index_ : if_then<not_at<two<'.'>>, seq<one<'.'>, ig_s, fncall>> {};
	struct in_eps : eps {};
	struct un_eps : eps {};
	struct in_ctrl : seq<one<'.'>, in_eps, dot_ctrl> {};
	struct index : seq<star<unary>, fncall, star<_index_>, un_eps, sor<in_ctrl, seq<opt<inf>, in_eps>>> {};
	//struct range : seq<opt<two<','>, ig_s, index>, two<'.'>, ig_s, index> {};
		// this grammar interferes with the sequence grammar on the ',' causing duplicates to exist
	struct range_op : two<'.'> {};
	struct _range_ : seq<range_op, opt<index>> {};
	struct range : seq<index, opt<_range_>> {};
	struct control : sor<branch, loop, while_l, for_l, match_expr, jumps> {};
	//struct valexpr : seq<sor<k_mut, op, eps>, sor<control, binary>, ig_s, opt<one<';'>, ig_s>> {};
	struct valexpr : seq<sor<k_mut, eps>, sor<control, binary>, ig_s, opt<one<';'>, ig_s>> {};
	struct mod_use : seq<k_use, star<use_one, one<':'>>, use_final> {};
	struct impl_expr : seq<k_impl, name_path, disable<typ>, ig_s, opt<array>, ig_s, opt<scope>> {};
	struct expr : seq<sor<mod_use, impl_expr, assign, seq<opt<k_do>, valexpr>>, ig_s, opt<one<';'>, ig_s>> {};
	struct stmt : seq<star<annotation, ig_s>, sor<global_annotation, mod_dec, expr>> {};
	struct program : seq<ig_s, star<stmt>, eolf> {};

	#undef key
	#undef pstr
}
