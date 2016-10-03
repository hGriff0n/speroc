#![feature(plugin)]
#![plugin(oak)]

extern crate oak_runtime;
use oak_runtime::*;

// Start blog post from here
//   Convert over to building AST

mod ast;


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
    k_let = "let"                                   > mk_let
    k_def = "def"                                   > mk_def
    k_static = "static"                             > mk_static
    k_match = "match"
    k_if = "if"
    k_else = "else"
    k_use = "use"
    b_true = "true"                                 > mk_true
    b_false = "false"                               > mk_false

    vcontext = (k_let / k_def / k_static) ignore*

    //
    // Language Identifiers
    //
    var = ["a-z_"] ["a-zA-Z0-9_"]*                  > mk_binding
    typ = ["A-Z"] ["a-zA-Z0-9"]*                    > mk_binding
    op = ["&=:"]? ["!@#$%^&*?<>|`,/\\-=+"]+         > mk_operator
    binding = (var/typ/op) ignore*
    assign = vcontext binding "=" ignore* atom      > mk_assignment

    //
    // Language Atoms
    //
    hex = "0x" ["0-9a-fA-F"]+                       > mk_hex_literal
    bin = "0b" ["0-1"]+                             > mk_byte_literal
    num = ["0-9"]+ ("." ["0-9"]*)?                  > mk_num_literal
    string = "\"" (!"\"" "\\"? .)* "\""             > mk_string_literal
    character = "'" "\\"? . "'"                     > mk_char_literal
    atom = (hex / bin / num / character / string)   > mk_atom

    //
    // Language Expressions
    //
    expression = (atom ignore*)
    // program = ignore* expression+
    program = assign

    //
    // Parsing Structures
    //
    use ast::*;
    
    //
    // Parsing Actions
    //
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
    fn mk_atom(l: LiteralType) -> AtomType { AtomType::Literal(l) }

    fn mk_def() -> Keyword { Keyword::Def }
    fn mk_let() -> Keyword { Keyword::Let }
    fn mk_static() -> Keyword { Keyword::Static }

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

    // val will change in the future
    fn mk_assignment(vis: Keyword, binding: BindingType, val: AtomType) -> ExprType {
        ExprType::Assignment(vis, binding, val)
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