#![feature(plugin)]
#![plugin(oak)]

extern crate oak_runtime;
use oak_runtime::*;

mod ast;

/*
What's different from Pt 3
    Recongize Variables as values
    Rewrote Grammar and AST to improve parsing
    Adds pretty printing (don't do more than touch on this)
*/

// TODO (Pt 4)
//  Improve the grammar design
//  Find a way to add in maintain '.' in the ast for function resolution lookup
//  Should I just leave all things that pass through FnCall as Functions (I can remove this in type checking)


// http://hyc.io/rust-lib/oak/index.html
grammar! spero_grammar {
    #![show_api]
    multiline = "##" (!"##" .)* "##" -> (^)
    comment = "#" (!"\n" .)* "\n" -> (^)
    spacing = [" \n\r\t"]+ -> (^)
    ignore = multiline / comment / spacing

    //
    // Language Keywords
    //
    k_let = "let" ignore+                                                               > mk_let
    k_def = "def" ignore+                                                               > mk_def
    k_static = "static" ignore+                                                         > mk_static
    k_mut = "mut" ignore+                                                               > mk_mut
    k_mod = "mod" ignore+
    k_match = "match" ignore+
    k_if = "if" ignore+
    k_else = "else" ignore+
    k_use = "use" ignore+
    b_true = "true" ignore+                                                             > mk_true
    b_false = "false" ignore+                                                           > mk_false
    keyword = ("let" / "def" / "static" / "mut" / k_match /
                k_if / k_else / k_use / "false" / "true") -> (^)
    vcontext = k_let / k_def / k_static

    //
    // Language Identifiers
    //
    var = !keyword ["a-z_"] ["a-zA-Z0-9_"]* ignore*                                     > mk_binding
    typ = ["A-Z"] ["a-zA-Z0-9"]* ignore*                                                > mk_binding
    op = ["&=:"]? ["!@#$%^&*?<>|`/\\-=+"]+ ignore*                                      > mk_operator
    tbinding = "(" ignore* var ("," ignore* var)* ")" ignore*                           > mk_tup_binding
    binding = tbinding / var / op / typ

    //
    // Function/Etc Definitions
    //
    typ_inf = "::" ignore* k_mut? typ                                                   > mk_type_inf
    assign_val = "=" ignore* k_mut? expr
    gen_var = var typ_inf
    gen_typ = typ typ_inf?
    gen_inst = gen_var / gen_typ
    gen_def = "[" ignore* gen_inst ("," ignore* gen_inst)* "]"
    tup_expr = expr typ_inf? assign_val?                                                > mk_tuple_elem

    //
    // Module Syntax
    //
    // mod_name = var ignore*
    // mod_dec = k_mod mod_name (":" ignore* mod_name)*                                 > mk_mod_dec
    
    // imp_star = "*" ignore*
    // imp_fmult = "{" mod_name ("," ignore* mod_name)* imp_rename? "}" ignore*
    // imp_emult = "{" binding ("," ignore* binding)* imp_rename? "}" ignore*
    // imp_front = imp_fmult / mod_name
    // imp_end = imp_emult / binding / imp_star
    // imp_rename = "as" ignore* binding ("," ignore* binding)*
    // mod_imp = k_use (imp_front ":" ignore*)* imp_end imp_rename?                     > mk_mod_import

    //
    // Language Atoms
    //
    hex = "0x" ["0-9a-fA-F"]+                                                           > mk_hex_literal
    bin = "0b" ["0-1"]+                                                                 > mk_byte_literal
    num = ["0-9"]+ ("." ["0-9"]+)?                                                      > mk_num_literal
    string = "\"" (!"\"" "\\"? .)* "\""                                                 > mk_string_literal
    character = "'" "\\"? . "'"                                                         > mk_char_literal
    tuple = "(" ignore* (tup_expr ("," ignore* tup_expr)*)? ")"                         > mk_tuple
    fnliteral = tuple "->" ignore* (typ scope) / expr
    literal = (hex / bin / num / character 
          / string / b_true / b_false) ignore*                                          > mk_atom
    name = (var / typ / op)                                                             > mk_var
    atom = literal / name

    //
    // Language Expressions
    //
    fncall = atom tuple? ignore*                                                        > mk_fncall
    dotflip = fncall ("." ignore* fncall)*                                              > mk_dotflip
    binop = dotflip (op ignore* dotflip)?                                               > mk_op_inv
    scope = "{" ignore* expr* ignore* "}" ignore*                                       > mk_scope
    assign = vcontext binding assign_val                                                > mk_assign
    // expr = mod_dec / scope / assign / binop / atom
    expr = scope / assign / binop
    // program = ignore* expr*
    program = fnliteral

    //
    // Parsing Structures
    //
    use ast::*;
    
    //
    // Parsing Actions
    //

    // Literals
    fn mk_num_literal(int_part: Vec<char>, float_part: Option<Vec<char>>) -> LiteralType {
        LiteralType::Num(int_part.into_iter().collect(), float_part.map(|a| a.into_iter().collect()))
    }
    fn mk_byte_literal(val: Vec<char>) -> LiteralType {
        LiteralType::Byte(val.into_iter().collect(), true)
    }
    fn mk_hex_literal(val: Vec<char>) -> LiteralType {
        LiteralType::Byte(val.into_iter().collect(), false)
    }
    fn mk_string_literal(val: Vec<char>) -> LiteralType {
        LiteralType::Str(val.into_iter().collect())
    }
    fn mk_char_literal(val: char) -> LiteralType {
        LiteralType::Char(val)
    }
    fn mk_true() -> LiteralType { LiteralType::Bool(true) }
    fn mk_false() -> LiteralType { LiteralType::Bool(false) }
    fn mk_atom(l: LiteralType) -> ExprType { ExprType::Literal(l) }
    fn mk_tuple(args: Option<(ExprType, Vec<ExprType>)>) -> ExprType {
        ExprType::Tuple(
            match args {
                Some(tup) => {
                    let mut vec = vec![tup.0];
                    let mut other = tup.1;
                    vec.append(&mut other);
                    vec
                },
                None => Vec::new(),
            }
        )
    }

    // Keywords
    fn mk_def() -> Keyword { Keyword::Def }
    fn mk_let() -> Keyword { Keyword::Let }
    fn mk_static() -> Keyword { Keyword::Static }
    fn mk_mut() -> Keyword { Keyword::Mut }

    // Statements
    // fn mk_mod_dec(base: BindingType, mut tail: Vec<BindingType>) -> ExprType {
    //     tail.insert(0, base);
    //     ExprType::ModDecl(tail)
    // }

    // Bindings
    fn mk_binding(c: char, mut name: Vec<char>) -> BindingType {
        name.insert(0, c);

        match c.is_uppercase() {
            true => BindingType::Type(name.into_iter().collect()),
            false => BindingType::Variable(name.into_iter().collect())
        }
    }
    fn mk_operator(c: Option<char>, mut rest: Vec<char>) -> BindingType {
        if c.is_some() {
            rest.insert(0, c.unwrap());
        }

        BindingType::Operator(rest.into_iter().collect())
    }
    fn mk_tup_binding(fst: BindingType, mut tail: Vec<BindingType>) -> BindingType {
        tail.insert(0, fst);
        BindingType::Tuple(tail)
    }
    fn mk_var(v: BindingType) -> ExprType {
        ExprType::Name(v)
    }

    // Expressions
    fn mk_fncall(callee: ExprType, args: Option<ExprType>) -> ExprType {
        use ast::ExprType::*;

        match args {
            Some(expr) => match expr {
                Tuple(mut exprs) => {
                    FnCall(Box::new(callee), exprs)
                },
                expr => {
                    FnCall(Box::new(callee), vec![expr])
                }
            },
            None => match callee {
                Name(_) => FnCall(Box::new(callee), vec![]),
                _ => callee,
            }
        }
    }
    fn mk_dotflip(callee: ExprType, mut dot_fn: Vec<ExprType>) -> ExprType {
        if dot_fn.len() == 0 {
            return callee;
        }

        let dot = dot_fn.remove(0);
        match dot {
            ExprType::FnCall(fun, mut args) => {
                args.insert(0, callee);
                mk_dotflip(ExprType::FnCall(fun, args), dot_fn)
            },
            _ => {
                panic!("Something went wrong in mk_dotflip");
            }
        }
    }
    fn mk_op_inv(lhs: ExprType, op: Option<(BindingType, ExprType)>) -> ExprType {
        match op {
            Some((op, rhs)) => ExprType::Operator(op, Box::new(lhs), Box::new(rhs)),
            None => lhs
        }
    }
    fn mk_scope(exprs: Vec<ExprType>) -> ExprType {
        ExprType::Scope(exprs)
    }
    fn mk_assign(key: Keyword, bind: BindingType, is_mut: Option<Keyword>, val: ExprType) -> ExprType {
        ExprType::Assignment(key, bind, is_mut.is_some(), Box::new(val))
    }
}

fn main() {
    use oak_runtime::parse_state::ParseResult::*;
    use ast::PrettyPrint;

    println!("");

    let input = "(x :: Int) -> x + 3";
    // let input = "let val = (x :: Int) -> x + 3";
    // let input = "let val = (x :: Int) { x + 3 Int(5) }"
    // let input = "let x = a(3 + 4, 4).+(4) + 3";
    // println(a(3, 4), 4)

    match spero_grammar::parse_program(input.into_state()).into_result() {
        Success(res) => {
            for result in &res {
                result.print(0);
            }
        },
        _ => println!("Unmatched")
    }

    println!("");
}
