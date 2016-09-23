#[macro_use]
extern crate pest;

use pest::prelude::*;

// http://dragostis.github.io/pest/pest/macro.grammar!.html
// http://dragostis.github.io/pest/pest/macro.process!.html
// https://github.com/W4RH4WK/Cleaver/blob/c9e1f1a8fbf3a26ebe1b3d8bad78079f017ed9e0/src/front/pest/mod.rs

/*
["a"]       - a
[i"a"]      - a | A
['a'..'z']  - [a..z]
a           - <a>
a ~ b       - <a><b>
a | b       - <a>|<b>
a*          - <a>*
a+          - <a>+
a?          - <a>?
&a          - match a no progress
!a          - match if a doesn't match without progress
[push(a)]   - matches a and pushes it's captured string on the stack
[pop()]     - pop from the stack and matches it
[peek()]    - peeks from the stack

Precedence climbing
expression = _{
    {}
    rule1               <- has higher precedence than the primary rule
    rul2 = {< pow }     <- right associativity
}
*/

impl_rdp! {
    grammar! {
        // precedence climbing
        expression = _{ // rule is silent because it's the rule we're matching
            { ["("] ~ expression ~ [")"] | number | literal } // primary
        }

        whitespace = _{ [" "] } // whitespce gets run between all rules

        /*
            Spero Literals
         */

        // Number Literals
        digit = _{ ['0'..'9'] }
        decimal = @{ ["-"]? ~ digit+ }
        hex = @{ ["0x"] ~ (digit | ['a'..'f'] | ['A'..'F'])+ }
        binary = @{ ["b"] ~ ['0'..'1']+ }
        float = @{ digit+ ~ ["."] ~ digit* }
        number = _{ hex | binary | float | decimal }

        // Other Literals
        str_char = _{ ["\\"]? ~ any }
        string = { ["\""] ~ (!["\""] ~ str_char)* ~ ["\""] }
        bools = @{ ["false"] | ["true"] }
        character = @{ ["'"] ~ str_char ~ ["'"] }       // Why can't I make this atomic ???
        literal = _{ string | bools | character }

        // ab = @{ a ~ b }                 // There can't be anything between a and b for ab to match (atomic)
        // atomic = @{ non_atomic }        // Atomic is cascading (rules called by atomics are themselves atomic)
        // non_atomic = !@{ a ~ b }        // Except if a rule is a non-atomic

        // comment = _{}
        // whitespace = _{ [" \t\n"] }     // whitespce gets run between all rules
    }

    process! {
        parse(&self) -> () { // return an i32 in the end 
            (&number: decimal) => {
                println!("Found (Int): {}", number);
            },
            (&number: hex) => {
                println!("Found (Hex): {}", number);
            },
            (&number: float) => {
                println!("Found (Float): {}", number);
            },
            (&number: binary) => {
                println!("Found (Binary): {}", number);
            },
            (&string: string) => {
                println!("Found (String): {}", string);
            },
            (&ch: character) => {
                println!("Found (Char): {}", ch);
            },
            (&b: bools) => {
                println!("Found (Bool): {}", b);
            }
        }
    }
}

fn main() {
    println!("");

    let mut parser = Rdp::new(StringInput::new("117.5"));
    parser.expression();
    parser.parse();

    let mut parser = Rdp::new(StringInput::new("b0101"));
    parser.expression();
    parser.parse();

    let mut parser = Rdp::new(StringInput::new("'a'"));
    parser.expression();
    parser.parse();

    println!("");
}