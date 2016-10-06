#![feature(plugin)]
#![plugin(oak)]

extern crate oak_runtime;
use oak_runtime::*;

/*
What's different from Pt 1
  Converts over to using an AST
    Explain why an ast is needed for parsing
  Tuples
    Why does "(3, 3 + 3)" not parse with the first grammar ???
      ',' is considered a valid operator
  Fn Calls

  NOTE: Bring in some stuff from pt3 and pt4
    Maybe move AST into pt4
*/

// TODO: Merge grammar aspects of pt2, pt3, and pt4
  // Develop the grammar to the point of straining (leave as an exercise?)
  // Move AST over into pt3 (with the complete grammar)

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
    k_let = "let" ignore*                                           > mk_let
    k_def = "def" ignore*                                           > mk_def
    k_static = "static" ignore*                                     > mk_static
    k_mut = "mut" ignore*                                           > mk_mut
    k_match = "match" ignore*
    k_if = "if" ignore*
    k_else = "else" ignore*
    k_use = "use" ignore*
    b_true = "true" ignore*                                         > mk_true
    b_false = "false" ignore*                                       > mk_false

    //
    // Language Binding
    //
    var = ["a-z_"] ["a-zA-Z0-9_"]*                                  > mk_binding
    typ = ["A-Z"] ["a-zA-Z0-9"]*                                    > mk_binding
    op = ["&=:"]? ["!@#$%^&*?<>|`,/\\-=+"]+                         > mk_operator
    binding = (var / typ / op) ignore*
    fn_name = (var / op / typ)

    //
    // Language Atoms
    //
    hex = "0x" ["0-9a-fA-F"]+                                       > mk_hex_literal
    bin = "0b" ["0-1"]+                                             > mk_byte_literal
    num = ["0-9"]+ ("." ["0-9"]*)?                                  > mk_num_literal
    string = "\"" (!"\"" "\\"? .)* "\""                             > mk_string_literal
    character = "'" "\\"? . "'"                                     > mk_char_literal
    tuple = "(" ignore* (expr ("," ignore* expr)*)? ")"             > mk_tuple
    fncall = fn_name tuple ignore*                                  > mk_fncall
    atom = (hex / bin / num / character / string)                   > mk_atom

    //
    // Language Expressions
    //
    expression = (atom ignore*)
    program = ignore* expression+

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
    fn mk_atom(l: LiteralType) -> AtomType { AtomType::Literal(l) }
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
}

fn main() {
    use oak_runtime::parse_state::ParseResult::*;

    println!("");

    let input = "5 ## Hello\n World ##  \"String\"";

    match spero_grammar::parse_program(input.into_state()).into_result() {
        Success(res) => println!("{:?}", res),
        _ => println!("Unmatched")
    }

    println!("");
}