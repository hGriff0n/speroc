#![feature(plugin)]
#![plugin(oak)]

extern crate oak_runtime;
use oak_runtime::*;

mod ast;

/*
What's different from the last part
    Completed Grammar (more or less)
    Start developing the ast
      describe why we need the ast
*/

// http://hyc.io/rust-lib/oak/index.html
grammar! spero_grammar {
    multiline = "##" (!"##" .)* "##"        -> (^)
    comment = "#" (!"\n" .)* "\n"           -> (^)
    whitespace = [" \n\r\t"]+               -> (^)
    ig = multiline / comment / whitespace   -> (^)
    oparen = "(" ig*
    cparen = ")" ig*
    obrack = "[" ig*
    cbrack = "]" ig*
    obrace = "{" ig*
    cbrace = "}" ig*

    //
    // Language Keywords
    //
    k_let = "let" ig+                                                                                   > mk_let
    k_def = "def" ig+                                                                                   > mk_def
    k_static = "static" ig+                                                                             > mk_static
    k_mut = "mut" ig+                                                                                   -> (^)
    k_mod = "mod" ig+                                                                                   -> (^)
    k_match = "match" ig+                                                                               -> (^)
    k_if = "if" ig+                                                                                     -> (^)
    k_elsif = "elsif" ig+                                                                               -> (^)
    k_else = "else" ig+                                                                                 -> (^)
    k_use = "use" ig+                                                                                   -> (^)
    b_true = "true" ig+                                                                                 > mk_true
    b_false = "false" ig+                                                                               > mk_false
    keyword = "let" / "def" / "static" / k_mut / k_match /
               k_if / k_else / k_use / "true" / "false"                                                 -> (^)
    vcontext = k_let / k_def / k_static

    //
    // Language Bindings
    //
    variable = var ig*
    typ = ["A-Z"] ["a-zA-Z_"]*                                                                          > mk_typ
    var = !keyword ["a-z_"] ["a-zA-Z0-9"]*                                                              > mk_var
    op = ["&=:"]? ["!@#$%^&*?<>|`/\\-=+"]+ ig*                                                          > mk_op
    type_nm = mod_spec typ array? ("&"/"*")? ig*                                                        > mk_full_typ
    mod_spec = ((var / typ) ":")*

    //
    // Language Literals
    //
    array = obrack cbrack   > mk_array  // tmp for type_nm

    program = typ

    use ast::*;

    // Keywords
    fn mk_let() -> Keyword { Keyword::Let }
    fn mk_def() -> Keyword { Keyword::Def }
    fn mk_static() -> Keyword { Keyword::Static }
    fn mk_true() -> Literal { Literal::Bool(true) }
    fn mk_false() -> Literal { Literal::Bool(false) }

    // Bindings
    fn mk_var(fst: char, mut tail: Vec<char>) -> Binding {
        tail.insert(0, fst);
        Binding::Variable(tail.clone())
    }
    fn mk_typ(fst: char, mut tail: Vec<char>) -> Binding {
        tail.insert(0, fst);
        Binding::Type(tail.clone())
    }
    fn mk_op(fst: Option<char>, mut tail: Vec<char>) -> Binding {
        if fst.is_some() {
            tail.insert(0, fst.unwrap());
        }
        Binding::Operator(tail.clone())
    }
    fn mk_full_typ(mod_path: Vec<Binding>, typ: Binding, arr: Option<Expression>) -> Binding {
        match typ {
            Binding::Type(ref name) => Binding::FullType(name.clone(), mod_path.clone(), arr),
            _ => panic!("Bad match in mk_full_typ"),
        }
    }

    // Literals
    fn mk_array() -> Expression { Expression::Array(Vec::new()) }
}

// disable(mut) is accepted (I don't know how)
// The type_assign grammar is slightly different than originally planned
// I'm not sure bindings (especially types) are correct (generics)

fn main() {
    use oak_runtime::parse_state::ParseResult::*;
    // use ast::PrettyPrint;

    println!("");

    let input = "a";

    println!("Parsing \"{}\"", input);
    match spero_grammar::parse_program(input.into_state()).into_result() {
        Success(res) => {
            // for result in &res {
            //     result.print(0);
            // }
            println!("Success")
        },
        _ => println!("Unmatched")
    }

    println!("");
}
