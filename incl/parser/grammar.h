#pragma once

#include "pegtl.hh"

namespace spero {
	namespace parser {
		namespace grammar {
			using namespace pegtl;
			#define pstr(x) pegtl_string_t((x))

			// Forward Declarations
			struct array;
			struct expr;
			struct scope;
			struct atom;
			struct pattern;

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

			//
			// Language Keywords
			//
			template<class T>
			struct key : seq<T, plus<ig>> {};
			struct k_let : key<pstr("let")> {};
			struct k_def : key<pstr("def")> {};
			struct k_static : key<pstr("static")> {};
			struct k_mut : key<pstr("mut")> {};
			struct k_mod : key<pstr("mod")> {};
			struct k_match : key<pstr("match")> {};
			struct k_if : key<pstr("if")> {};
			struct k_elsif : key<pstr("elsif")> {};
			struct k_else : key<pstr("else")> {};
			struct k_use : key<pstr("use")> {};
			struct b_false : key<pstr("false")> {};
			struct b_true : key<pstr("true")> {};

			struct vcontext : sor<k_let, k_def, k_static> {};
			struct keyword : sor<vcontext, k_mut, k_mod, k_match, k_if, k_elsif, k_else, k_use, b_false, b_true> {};

			//
			// Language Bindings
			//
			struct var : seq<ranges<'a', 'z', '_'>, star<ascii::identifier_other>> {};
			struct typ : seq<range<'A', 'Z'>, star<ascii::identifier_other>> {};
			struct op : seq<opt<sor<one<'&'>, one<'='>, one<':'>>>,
				plus<sor<one<'!'>, one<'@'>, one<'#'>, one<'$'>, one<'%'>, one<'^'>, one<'&'>, one<'*'>, one<'?'>, one<'<'>,
				one<'>'>, one<'|'>, one<'`'>, one<'/'>, one<'\\'>, one<'-'>, one<'='>, one<'-'>, one<'+'>>>, ig_s> {};
			struct variable : seq<var, ig_s> {};
			struct name_path : star<sor<var, typ>, one<':'>> {};
			template<class T> struct qual_name : seq<name_path, T, ig_s> {};
			struct type : seq<name_path, typ, opt<array>, sor<one<'&'>, one<'*'>, eps>, ig_s> {};

			//
			// Language Literals
			//
			struct hex : seq<pstr("0x"), plus<xdigit>> {};
			struct bin : seq<pstr("0b"), plus<sor<one<'0'>, one<'1'>>>> {};
			struct num : seq<plus<digit>, opt<one<'.'>, plus<digit>>> {};
			struct str_body : until<one<'"'>, seq<opt<one<'\\'>>, any>> {};
			struct str : seq<one<'"'>, str_body> {};
			struct character : seq<one<'\''>, opt<one<'\\'>>, any, one<'\''>> {};
			struct tuple : seq<oparen, opt<sequ<expr>>, cparen> {};
			struct array : seq<obrack, opt<sequ<expr>>, cbrack> {};
			struct fn_def : sor<scope, seq<pstr("->"), ig_s, sor<seq<type, ig_s, scope>, seq<opt<one<'.'>>, expr>>>> {};
			struct fn_or_tuple : sor<seq<one<'.'>, expr>, seq<tuple, opt<fn_def>>> {};
			struct lit : sor<hex, bin, num, str, character, fn_or_tuple> {};

			//
			// Language Atoms
			//
			struct varname : sor<qual_name<var>, qual_name<typ>, op> {};
			struct fncall : seq<varname, opt<array>, opt<tuple>> {};
			struct scope : seq<obrace, star<expr>, cbrace> {};
			struct pat_tuple : seq<oparen, sequ<pattern>, one<')'>> {};
			struct pattern : sor<placeholder, var, pat_tuple, seq<typ, opt<pat_tuple>>> {};
			struct case_stmt : seq<sequ<seq<pattern, ig_s>>, pstr("->"), ig_s, expr> {};
			struct match_expr : seq<k_match, opt<atom>, obrace, plus<case_stmt>, cbrace> {};
			struct atom : seq<sor<match_expr, fncall, scope, lit>, ig_s> {};

			//
			// Language Decorators
			//
			// anexpr = expr / keyword
			// antuple = oparen (anexpr ("," ig* anexpr)*)? cparen
			struct annotation : seq<one<'@'>, var, opt<tuple>> {};
			struct mod_dec : seq<k_mod, list<var, one<':'>>, ig_s> {};
			struct mut_type : seq<opt<k_mut>, type> {};
			struct type_tuple : seq<oparen, sequ<mut_type>, cparen> {};
			struct inf : seq<pstr("::"), ig_s, opt<type_tuple, ig_s, pstr("->"), ig_s>, mut_type> {};
			struct gen_type : seq<type, sor<one<'+'>, one<'-'>, eps>, ig_s, opt<sor<pstr("::"), pstr("!:"), one<'>'>, one<'<'>>, ig_s, type>> {};
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
			struct index : seq<atom, star<one<'.'>, ig_s, atom>, opt<inf>> {};
			struct binary : seq<index, star<op, index>> {};
			struct branch : seq<k_if, expr, expr, star<k_elsif, expr, expr>, if_must<k_else, expr>> {};
			struct use_path : seq<list<sor<var, seq<obrace, sequ<variable>, cbrace>>, one<':'>>, one<':'>> {};
			struct use_elem : sor<seq<variable, opt<pstr("as"), ig_s, variable>>, seq<typ, ig_s, opt<pstr("as"), ig_s, typ, ig_s>>> {};
			struct mod_use : seq<k_use, opt<use_path>, sor<one<'*'>, opt<obrace, sequ<use_elem>, cbrace>, use_elem>> {};
			struct expr : seq<sor<mod_use, assign, branch, binary>, ig_s> {};
			struct line : sor<mod_dec, expr> {};
			struct program : seq<ig_s, star<line>, eof> {};

			#undef pstr
		}
	}
}