#pragma once
#pragma warning(disable : 4503)

#include "pegtl.hh"

namespace spero::parser::grammar {
	using namespace pegtl;
	#define pstr(x) pegtl_string_t((x))
	#define key(x) seq<pstr((x)), not_at<ascii::identifier_other>, ig_s>

	// Forward Declarations
	struct array; struct expr; struct scope; struct atom; struct pattern;

	// Ignore Characters
	struct one_line_comment : seq<one<'#'>, until<eolf>> {};
	struct multiline_comment : seq<two<'#'>, until<two<'#'>>> {};
	struct whitespace : plus<space> {};
	struct ig : sor<multiline_comment, one_line_comment, whitespace> {};
	struct ig_s : star<ig> {}; 
	struct eps : success {};

	// Special Characters
	template<char ch>
	struct spec_char : seq<one<ch>, star<ig>> {};
	struct obrace : spec_char<'{'> {};
	struct cbrace : spec_char<'}'> {};
	struct obrack : spec_char<'['> {};
	struct cbrack : spec_char<']'> {};
	struct oparen : spec_char<'('> {};
	struct cparen : spec_char<')'> {};
	struct placeholder : one<'_'> {};

	// Helper Structs
	template<class Rule, char sep = ','>
	struct sequ : list<seq<ig_s, Rule>, one<sep>> {};
	template<class Rule, class Must>
	struct if_then : if_then_else<Rule, Must, failure> {};

	//
	// Language Keywords
	//
	struct k_let : key("let") {};
	struct k_def : key("def") {};
	struct k_static : key("static") {};
	struct k_mut : key("mut") {};
	struct k_mod : key("mod") {};
	struct k_match : key("match") {};
	struct k_if : key("if") {};
	struct k_elsif : key("elsif") {};
	struct k_else : key("else") {};
	struct k_use : key("use") {};
	struct vcontext : sor<k_let, k_def, k_static> {};
	struct keyword : sor<vcontext, k_mut, k_mod, k_match, k_if, k_elsif, k_else, k_use> {};

	//
	// Language Bindings
	//
	struct var : seq<ranges<'a', 'z', '_'>, star<ascii::identifier_other>> {};
	struct typ : seq<range<'A', 'Z'>, star<ascii::identifier_other>> {};
	struct op : seq<opt<sor<one<'&'>, one<'='>, one<':'>>>,
					plus<sor<one<'!'>, one<'@'>, one<'#'>, one<'$'>, one<'%'>, one<'^'>, one<'&'>, one<'*'>, one<'?'>, one<'<'>,
				                one<'>'>, one<'|'>, one<'`'>, one<'/'>, one<'\\'>, one<'-'>, one<'='>, one<'-'>, one<'+'>>>, ig_s> {};
	struct variable : seq<var, ig_s> {};
	struct name_path_part : if_then<sor<var, typ>, one<':'>> {};
	struct name_path : star<name_path_part> {};
	template<class T> struct qual_name : seq<name_path, T, ig_s> {};
	struct typ_gen_inst : opt<array> {};
	struct typ_pointer : sor<one<'&'>, one<'*'>, eps> {};
	struct type : seq<name_path, typ, typ_gen_inst, typ_pointer, ig_s> {};
	struct varname : sor<qual_name<var>, qual_name<typ>, op> {};

	//
	// Language Literals
	//
	struct hex_body : plus<xdigit> {};
	struct hex : seq<pstr("0x"), hex_body> {};
	struct bin_body : plus<sor<one<'0'>, one<'1'>>> {};
	struct bin : seq<pstr("0b"), bin_body> {};
	struct dec : seq<plus<digit>, one<'.'>, plus<digit>> {};
	struct num : plus<digit> {};
	struct str_body : until<one<'"'>, seq<opt<one<'\\'>>, any>> {};
	struct str : seq<one<'"'>, str_body> {};
	struct character : seq<one<'\''>, opt<one<'\\'>>, any, one<'\''>> {};
	struct b_false : seq<pstr("false"), not_at<ascii::identifier_other>> {};
	struct b_true : seq<pstr("true"), not_at<ascii::identifier_other>> {};
	struct tuple : seq<oparen, opt<sequ<expr>>, cparen> {};
	struct array : seq<obrack, opt<sequ<expr>>, cbrack> {};
	//struct fn_scope_only : seq<scope> {};
	struct fn_rettype : seq<type, ig_s, scope> {};
	struct fn_one_line : seq<opt<one<'.'>>, expr> {};
	struct fn_def : sor<scope, seq<pstr("->"), ig_s, sor<fn_rettype, fn_one_line>>> {};
	struct fn_forward : seq<one<'.'>, expr> {};
	struct fn_opt : opt<fn_def> {};
	struct fn_or_tuple : sor<fn_forward, seq<tuple, fn_opt>> {};
	struct lit : sor<hex, bin, dec, num, str, character, b_false, b_true, array, fn_or_tuple> {};

	//
	// Language Atoms
	//
	struct fncall : seq<varname, opt<array>, opt<tuple>> {};
	struct scope : seq<obrace, star<expr>, cbrace> {};
	struct pat_tuple : seq<oparen, sequ<pattern>, one<')'>> {};
	struct pattern : sor<placeholder, var, pat_tuple, seq<typ, opt<pat_tuple>>> {};
	struct case_stmt : seq<sequ<seq<pattern, ig_s>>, pstr("->"), ig_s, expr> {};
	struct match_expr : seq<k_match, opt<atom>, obrace, plus<case_stmt>, cbrace> {};
	struct atom : seq<sor<match_expr, lit, fncall, scope>, ig_s> {};

	//
	// Language Decorators
	//
	// anexpr = expr / keyword
	// antuple = oparen (anexpr ("," ig* anexpr)*)? cparen
	struct annotation : seq<one<'@'>, var, opt<tuple>> {};
	struct mod_dec : seq<k_mod, list<var, one<':'>>, ig_s> {};
	struct mut_type : seq<opt<k_mut>, type> {};
	struct type_tuple : seq<oparen, sequ<mut_type>, cparen> {};
	struct inf_fn_type : opt<type_tuple, ig_s, pstr("->"), ig_s> {};
	struct inf : seq<pstr("::"), ig_s, inf_fn_type, mut_type> {};
	struct gen_variance : sor<one<'+'>, one<'-'>, eps> {};
	struct gen_subtype : seq<sor<pstr("::"), pstr("!:"), one<'>'>, one<'<'>>, ig_s, type> {};
	struct gen_type : seq<typ, gen_variance, ig_s, opt<gen_subtype>> {};
	struct gen_val : seq<variable, opt<pstr("::"), ig_s, type>, opt<one<'='>, ig_s, expr>> {};
	struct gen_part : sor<gen_type, gen_val> {};
	struct generic : seq<obrack, sequ<gen_part>, cbrack> {};

	//
	// Assignment Grammar
	//
	struct adt_con : seq<typ, opt<type_tuple>, ig_s> {};
	struct cons : sor<adt_con, tuple> {};
	struct constructors : sequ<cons, '|'> {};
	struct var_assign : seq<sor<pattern, op>, ig_s, opt<inf>, one<'='>, ig_s, opt<k_mut>, expr> {};
	struct type_assign : seq<typ, ig_s, opt<generic>, one<'='>, ig_s, opt<constructors>, scope> {};
	struct assign : seq<vcontext, sor<type_assign, var_assign>> {};

	//
	// Language Expressions
	//
	struct _index_ : seq<one<'.'>, ig_s, atom> {};
	struct index : seq<atom, star<_index_>, opt<inf>> {};
	struct _binary_ : seq<op, index> {};
	struct binary : seq<index, star<_binary_>> {};
	struct branch : seq<k_if, expr, expr, star<k_elsif, expr, expr>, if_must<k_else, expr>> {};
	struct use_path_elem : sor<var, seq<obrace, sequ<variable>, cbrace>> {};
	struct use_path : seq<list<use_path_elem, one<':'>>, one<':'>> {};
	struct use_elem : sor<seq<variable, opt<pstr("as"), ig_s, variable>>, seq<typ, ig_s, opt<pstr("as"), ig_s, typ, ig_s>>> {};
	struct mod_use : seq<k_use, opt<use_path>, sor<one<'_'>, opt<obrace, sequ<use_elem>, cbrace>, use_elem>> {};
	struct expr : seq<sor<mod_use, assign, branch, binary>, ig_s> {};
	struct line : sor<mod_dec, expr> {};
	struct program : seq<ig_s, star<line>, eof> {};

	#undef key
	#undef pstr
}
