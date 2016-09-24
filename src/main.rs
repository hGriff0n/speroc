#![feature(plugin)]
#![plugin(oak)]

extern crate oak_runtime;
use oak_runtime::*;

grammar! spero_grammar {
    //
    // Language Keywords
    //
    boolean = "true" / "false"
    vcontext = "let" / "def" / "static"
    keyword = vcontext / "match" / "if" / "else" / "mod" / "use"

    multiline = "##" (!"##" .)* "##"
    comment = "#" (!"\n" .)* "\n"

    variable = ["a-z_"] ["a-zA-Z0-9_"]*
    type = ["A-Z"] ["a-zA-Z0-9"]*
    op = ["&="] ["!@#$%^&*?<>[]~`-=+"]*

    //
    // Language Atoms
    //

    // Basic Literals
    decimal = ["0-9"]+
    hex = "0x" ["0-9a-fA-F"]+
    bin = "0b" ["0-1"]+
    float = decimal "." decimal?
    string = "\"" (!"\"" "\\"? .) "\""
    character = "'" "\\"? . "'"

    sum = number ("+" number)* > add
    number = ["0-9"]+ > to_number

    use std::str::FromStr;

    fn add(x: u32, rest: Vec<u32>) -> u32 {
        rest.iter().fold(x, |x, y| x+y)
    }

    fn to_number(raw_text: Vec<char>) -> u32 {
        let text: String = raw_text.into_iter().collect();
        u32::from_str(&*text).unwrap()
    }
}

fn main() {
    let state = spero_grammar::parse_sum("7+2+1".into_state());
    println!("{}", state.unwrap_data());
}