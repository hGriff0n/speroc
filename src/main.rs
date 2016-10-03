#![feature(plugin)]
#![plugin(oak)]

extern crate oak_runtime;
use oak_runtime::*;

mod ast;

/*
What's different from Pt 2
    Adds assignment and binary operations
    Improves grammar organization (todo)
    Adds modules and dot-flipping syntax
    Improves tuple support and recognition
    Adds pretty printing (don't do more than touch on this)
*/

// TODO (Pt 3)
//  Add in grammar for dot-flipping
//  Add in grammar for tuple deconstruction in bindings
//  Prevent keywords from being accepted as variables

// TODO (Pt 4)
//  Add in ast for module imports
//  Improve the grammar for functions (change to variable)

// TODO:
    // Add in pretty ast-printing


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
    k_let = "let" ignore*                                           > mk_let
    k_def = "def" ignore*                                           > mk_def
    k_static = "static" ignore*                                     > mk_static
    k_mut = "mut" ignore*                                           > mk_mut
    k_mod = "mod" ignore*
    k_match = "match" ignore*
    k_if = "if" ignore*
    k_else = "else" ignore*
    k_use = "use" ignore*
    b_true = "true" ignore*                                         > mk_true
    b_false = "false" ignore*                                       > mk_false
    keyword = ("let" / "def" / "static" / "mut" / k_match / k_if / k_else / k_use / "false" / "true") -> (^)
    vcontext = k_let / k_def / k_static

    //
    // Language Identifiers
    //
    var = ["a-z_"] ["a-zA-Z0-9_"]*                                  > mk_binding
    typ = ["A-Z"] ["a-zA-Z0-9"]*                                    > mk_binding
    // mdiv = ["*/"]                                   > mk_binop              // This goes in a separate post
    // splus = ["-+"]                                  > mk_binop
    op = ["&=:"]? ["!@#$%^&*?<>|`/\\-=+"]+                          > mk_operator
    var_op = (var / op) ignore*
    binding = (var / op / typ) ignore*
    // dbinding = "(" ignore* var_op ("," ignore* var_op)* ")" ignore* > mk_tup_binding

    //
    // Module Syntax
    //
    mod_name = var ignore*
    mod_dec = k_mod mod_name (":" ignore* mod_name)*                > mk_mod_dec
    
    // imp_star = "*" ignore*
    // imp_fmult = "{" mod_name ("," ignore* mod_name)* imp_rename? "}" ignore*
    // imp_emult = "{" binding ("," ignore* binding)* imp_rename? "}" ignore*
    // imp_front = imp_fmult / mod_name
    // imp_end = imp_emult / binding / imp_star
    // imp_rename = "as" ignore* binding ("," ignore* binding)*
    // mod_imp = k_use (imp_front ":" ignore*)* imp_end imp_rename?    > mk_mod_import

    //
    // Language Atoms
    //
    hex = "0x" ["0-9a-fA-F"]+                                       > mk_hex_literal
    bin = "0b" ["0-1"]+                                             > mk_byte_literal
    num = ["0-9"]+ ("." ["0-9"]*)?                                  > mk_num_literal
    string = "\"" (!"\"" "\\"? .)* "\""                             > mk_string_literal
    character = "'" "\\"? . "'"                                     > mk_char_literal
    tuple = "(" ignore* (expr ("," ignore* expr)*)? ")"             > mk_tuple
    literal = (hex / bin / num / character
          / string / b_true / b_false) ignore*                      > mk_atom
    fncall = binding tuple ignore*                                  > mk_fncall
    atom = literal / fncall / tuple ignore*
        //   / (var / type / op)

    //
    // Language Expressions
    //
    // fncall = ... ignore*
    // dotflip = atom ("." ignore* fncall)?
    // binop = dotflip (op ignore* dotflip)?
    // scope = "{" ignore* expression* ignore* "}" ignore*
    // valexpr = (scope / binop)
    // assign = vcontext binding "=" ignore* k_mut? valexpr
    // expression = (valexpr / assign) ignore*
    binop = atom op ignore* atom                                    > mk_op_inv
    scope = "{" ignore* expr* ignore* "}"                           > mk_scope
    assign = vcontext binding "=" ignore* k_mut? expr               > mk_assignment
    expr = (mod_dec / scope / assign / binop / atom) ignore*
    program = ignore* expr+

    // program = ignore* assign
    // program = num                                                   > mk_atom

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
    fn mk_fncall(fun: BindingType, args: ExprType) -> ExprType {
        ExprType::FnCall(fun, None, Some(Box::new(args)))
    }

    // Keywords
    fn mk_def() -> Keyword { Keyword::Def }
    fn mk_let() -> Keyword { Keyword::Let }
    fn mk_static() -> Keyword { Keyword::Static }
    fn mk_mut() -> Keyword { Keyword::Mut }

    // Modules
    fn mk_mod_dec(base: BindingType, mut tail: Vec<BindingType>) -> ExprType {
        tail.insert(0, base);
        ExprType::ModDecl(tail)
    }

    // Bindings
    fn mk_binding(c: char, mut name: Vec<char>) -> BindingType {
        name.insert(0, c);

        match c.is_uppercase() {
            true => BindingType::Type(name.into_iter().collect()),
            false => BindingType::Variable(name.into_iter().collect())
        }
    }

    fn mk_operator(c: Option<char>, mut rest: Vec<char>) -> BindingType {
        if c.is_none() {
            rest.insert(0, c.unwrap());
        }

        BindingType::Operator(rest.into_iter().collect())
    }

    fn mk_op_inv(lhs: ExprType, op: BindingType, rhs: ExprType) -> ExprType {
        ExprType::Binary(op, Box::new(lhs), Box::new(rhs))
    }

    fn mk_scope(exprs: Vec<ExprType>) -> ExprType {
        ExprType::Scope(exprs)
    }

    fn mk_assignment(vis: Keyword, binding: BindingType, is_mut: Option<Keyword>, val: ExprType) -> ExprType {
        ExprType::Assignment(vis, binding, is_mut.is_some(), Box::new(val))
    }
}

fn main() {
    use oak_runtime::parse_state::ParseResult::*;

    println!("");

    let input = "mod std:io:prin";

    match spero_grammar::parse_program(input.into_state()).into_result() {
        Success(res) => {
            for result in &res {
                println!("{}", result)
            }
        },
        _ => println!("Unmatched")
    }

    println!("");
}