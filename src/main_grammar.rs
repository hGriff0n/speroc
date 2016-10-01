#![feature(plugin)]
#![plugin(oak)]

extern crate oak_runtime;
use oak_runtime::*;

// Start blog post from here


// http://hyc.io/rust-lib/oak/index.html
grammar! spero_grammar {
    #![show_api]
    multiline = "##" (!"##" .)* "##" -> (^)
    comment = "#" (!"\n" .)* "\n" -> (^)
    spacing = [" \n\r\t"]* -> (^)
    ignore = multiline / comment / spacing

    //
    // Language Keywords
    //

    k_let = "let"
    k_def = "def"
    k_static = "static"
    k_match = "match"
    k_if = "if"
    k_else = "else"
    k_use = "use"
    b_true = "true"                                 > mk_true
    b_false = "false"                               > mk_false

    vcontext = k_let / k_def / k_static
    // keyword = vcontext / "match" / "if" / "else" / "mod" / "use"

    var = ["a-z_"] ["a-zA-Z0-9_"]*                  > mk_binding
    typ = ["A-Z"] ["a-zA-Z0-9"]*                    > mk_binding
    op = ["&=:"]? ["!@#$%^&*?<>|`,/\\-=+"]+         > mk_operator
    binding = var / typ / op
    // fn_sig = (<Type_List>) -> <Type>
    // type_inf = :: <type>

    //
    // Language Atoms
    //

    // Basic Literals
    digit = ["0-9"]
    hex = "0x" ["0-9a-fA-F"]+                       > mk_hex_literal
    bin = "0b" ["0-1"]+                             > mk_byte_literal
    float = digit+ "." digit*                       > mk_float_literal
    int = digit+                                    > mk_int_literal
    string = "\"" (!"\"" "\\"? .)* "\""             > mk_string_literal
    character = "'" "\\"? . "'"                     > mk_char_literal
    // tup_expr_list = (expression ignore "," ignore)* expression
    // tuple = "(" ignore tup_expr_list ignore ")"

    atom = (b_true / b_false / hex / bin / float / int / string / character) > run_expr
    // expression = "{" ignore expression+ "}" / atom ignore
    expression = atom ignore
    program = ignore expression+
    // program = binding
    
    //
    // Parsing Actions
    //
    fn mk_hex_literal(hex_text: Vec<char>) -> String {
        println!("Hex Literal: {:?}", hex_text);
        hex_text.into_iter().collect()
    }

    fn mk_byte_literal(byte_text: Vec<char>) -> String {
        println!("Byte Literal: {:?}", byte_text);
        byte_text.into_iter().collect()
    }

    fn mk_float_literal(mut int_part: Vec<char>, mut float_part: Vec<char>) -> String {
        println!("Float Literal: {:?}.{:?}", int_part, float_part);

        int_part.push('.');
        int_part.append(float_part.as_mut());
        int_part.into_iter().collect()
    }

    fn mk_int_literal(int_text: Vec<char>) -> String {
        println!("Int Literal: {:?}", int_text);
        int_text.into_iter().collect()
    }

    fn mk_string_literal(string: Vec<char>) -> String {
        println!("String Literal: {:?}", string);
        string.into_iter().collect()
    }

    fn mk_char_literal(c: char) -> String {
        println!("Char Literal: {}", c);

        c.to_string()
    }

    fn mk_true() -> String { String::from("true") }
    fn mk_false() -> String { String::from("false") }
    fn mk_operator(fst: Option<char>, mut op: Vec<char>) -> String {
        if fst.is_some() {
            op.insert(0, fst.unwrap());
        }
        
        println!("Operator: {:?}", op);
        op.into_iter().collect()
    }

    fn mk_binding(fst: char, mut last: Vec<char>) -> String {
        let is_type = fst.is_uppercase();

        last.insert(0, fst);

        println!("Binding ({}): {:?}", is_type, last);
        last.into_iter().collect()
    }

    fn run_expr(s: String) {
        println!("Expression: {}", s);
    }
}

fn main() {
    println!("");

    let input = "'a'";
    spero_grammar::parse_program(input.into_state());

    println!("");
}