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
    k_let = "let" ig+                                                                                   -> (^)
    k_def = "def" ig+                                                                                   -> (^)
    k_static = "static" ig+                                                                             -> (^)
    k_mut = "mut" ig+                                                                                   -> (^)
    k_mod = "mod" ig+                                                                                   -> (^)
    k_match = "match" ig+                                                                               -> (^)
    k_if = "if" ig+                                                                                     -> (^)
    k_elsif = "elsif" ig+                                                                               -> (^)
    k_else = "else" ig+                                                                                 -> (^)
    k_use = "use" ig+                                                                                   -> (^)
    b_true = "true" ig+                                                                                 -> (^)
    b_false = "false" ig+                                                                               -> (^)
    keyword = k_let / k_def / k_static / k_mut / k_match /
               k_if / k_else / k_use / b_true / b_false                                                 -> (^)
    vcontext = k_let / k_def / k_static                                                                 -> (^)

    //
    // Language Bindings
    //
    variable = var ig*
    typ = ["A-Z"] ["a-zA-Z0-9"]*                                                                        -> (^)
    var = !keyword ["a-z_"] ["a-zA-Z0-9_"]*                                                             -> (^)
    op = ["&=:"]? ["!@#$%^&*?<>|`/\\-=+"]+ ig*                                                          -> (^)
    type_nm = mod_spec typ array? ("&"/"*")? ig*                                                        -> (^)
    tup_decom = oparen variable ("," ig* variable)* cparen                                              -> (^)
    mod_spec = ((var / typ) ":")*                                                                       -> (^)
    full_nm = mod_spec (variable / (typ ig*))                                                           -> (^) 

    //
    // Language Literals
    //
    hex = "0x" ["0-9a-fA-F"]+                                                                           -> (^)
    bin = "0b" ["0-1"]+                                                                                 -> (^)
    num = ["0-9"]+ ("." ["0-9"]+)?                                                                      -> (^)
    string = "\"" (!"\"" "\\"? .)* "\""                                                                 -> (^)
    character = "'" "\\"? . "'"                                                                         -> (^)
    tuple = oparen (expr ("," ig* expr)*)? cparen                                                       -> (^)
    array = obrack (expr ("," ig* expr)*)? cbrack                                                       -> (^)
    name = full_nm / op                                                                                 -> (^)
    fn_def = scope / ("->" ig* ((type_nm scope) / ("."? expr)))                                         -> (^)
    fn_or_tuple = ("." expr) / (tuple fn_def?)                                                          -> (^)
    fncall = name array? tuple?                                                                         -> (^)
    literal = (hex / bin / num / string / fn_or_tuple / fncall / b_false / b_true / array) ig*          -> (^)

    //
    // Language Statements
    //
    // anexpr = expr / keyword
    // antuple = oparen (anexpr ("," ig* anexpr)*)? cparen
    annotation = "@" var tuple?                                                                         -> (^)
    mod_dec = k_mod var (":" var)* ig+                                                                  -> (^)

    //
    // Language Expression Parts
    //
    mut_type = k_mut? type_nm                                                                           -> (^)
    type_tuple = oparen (mut_type ("," ig* mut_type)*)? cparen                                          -> (^)
    gen = obrack gen_part ("," ig* gen_part)* cbrack                                                    -> (^)
    gen_part = (gen_type / gen_val) ig*                                                                 -> (^)
    gen_type = type_nm ("+"/"-")? (("::"/"!:"/"<"/">") ig* type_nm)?                                    -> (^)
    gen_val = variable ("::" ig* type_nm)? ("=" ig* expr)?                                              -> (^)
    mod_use = k_use (use_path ":")* ("*" / (obrace use_elem ("," ig* use_elem)+ cbrace) / use_elem)     -> (^)
    use_path = var / (obrace variable ("," ig* variable)+ cbrace)                                       -> (^)
    use_elem = (variable ("as" ig* variable)?) / (typ ig* ("as" ig* typ ig*)?)                          -> (^)
    inf = "::" ig* (type_tuple ig* "->" ig*)? mut_type                                                  -> (^) 

    //
    // Assignment
    //
    adt_con = typ type_tuple? ig*                                                                       -> (^)
    cons = adt_con / tuple                                                                              -> (^)
    constructors = (cons ("|" ig* cons)*)                                                               -> (^)
    var_assign = (variable / tup_decom /op) inf? "=" ig* k_mut? expr                                    -> (^)
    type_assign = typ ig* gen? "=" ig* constructors? scope                                              -> (^)
    assign = vcontext (type_assign / var_assign)                                                        -> (^)

    // Expressions
    atom = scope / case / literal                                                                       -> (^)
    index = atom ("." ig* atom)* inf?                                                                   -> (^)
    binary = index (op index)*                                                                          -> (^)
    scope = obrace expr* cbrace inf?                                                                    -> (^)
    branch = k_if expr expr (k_elsif expr expr)* (k_else expr)                                          -> (^)
    pattern = ("_" / var / (typ (oparen pattern ("," ig* pattern)* cparen)?)) ig*                       -> (^)
    case_stmt = pattern ("," ig* pattern)* "->" ig* expr                                                -> (^)
    case = k_match expr? obrace case_stmt+ cbrace                                                       -> (^)
    expr = annotation* ig* (assign / branch / binary / mod_use) ig*                                     -> (^)
    line = expr / mod_dec                                                                               -> (^)
    program = ig* line*                                                                                 -> (^)
}

// disable(mut) is accepted (I don't know how)
// The type_assign grammar is slightly different than originally planned
// case isn't in the correct position

fn main() {
    use oak_runtime::parse_state::ParseResult::*;
    // use ast::PrettyPrint;

    println!("");

    // let input = "v :: Num = 3";
    // let input = "(x :: T)";
    // let input = "(Foo)";
    // let input = "let val = (x :: Int) -> x + 3";
    // let input = "let val = (x :: Int) { x + 3 Int(5) }";
    // let input = "a(3 + 4)";
    let input = "def Foo = None | (x :: Int) {
        let bar = Array.new(x)

        def get = (i :: Int) -> bar[i]
    }";

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
