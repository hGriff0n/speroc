use std::iter;
use std::fmt;

pub enum Keyword {
    Let,
    Def,
    Static
}

pub enum Literal {
    Bool(bool),
}

#[derive(Clone)]
pub enum Binding {
    Variable(Vec<char>),
    Type(Vec<char>),
    Operator(Vec<char>),
    FullType(Vec<char>, Vec<Binding>, Option<Expression>),
}

#[derive(Clone)]
pub enum Expression {
    Empty,
    Array(Vec<Expression>),
}

pub trait PrettyPrint {
    fn print(&self, buf: usize);
}

fn mk_buffer(size: usize) -> String {
    iter::repeat(" ").take(size).collect()
}

// impl PrettyPrint for LiteralType {
//     fn print(&self, buf: usize) {
//         use self::LiteralType::*;
//         let buffer = mk_buffer(buf);

//         match *self {
//             Num(ref int_part, ref float_part) => {
//                 match *float_part {
//                     Some(ref flt) => println!("{}Float {}.{}", buffer, int_part, flt),
//                     None => println!("{}Int {}", buffer, int_part),
//                 }
//             },
//             Byte(ref string, is_hex) => println!("{}Byte({}) {}", buffer, is_hex, string),
//             Str(ref string) => println!("{}String {}", buffer, string),
//             Bool(boolean) => println!("{}Bool {}", buffer, boolean),
//             Char(ch) => println!("{}Char {}", buffer, ch),
//         }
//     }
// }

// impl PrettyPrint for BindingType {
//     fn print(&self, buf: usize) {
//         use self::BindingType::*;
//         let buffer = mk_buffer(buf);

//         match *self {
//             Variable(ref var) => println!("{}Variable {}", buffer, var),
//             Type(ref typ) => println!("{}Type {}", buffer, typ),
//             Operator(ref op) => println!("{}Op {}", buffer, op),
//             Tuple(ref binds) => {
//                 println!("{}Tuple Binding", buffer);

//                 for piece in binds {
//                     piece.print(buf + 1);
//                 }
//             }
//         }
//     }
// }

impl fmt::Display for Keyword {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        use self::Keyword::*;

        match *self {
            Def => write!(f, "def"),
            Let => write!(f, "let"),
            Static => write!(f, "static"),
        }
    }
}

// #[allow(unused_variables)]
// impl PrettyPrint for ExprType {
//     fn print(&self, buf: usize) {
//         use self::ExprType::*;
//         let buffer = mk_buffer(buf);

//         match *self {
//             Name(ref name) => name.print(buf),
//             Literal(ref liter) => liter.print(buf),
//             Tuple(ref elems) => {
//                 println!("{}Tuple[{}]", buffer, elems.len());

//                 for elem in elems {
//                     elem.print(buf + 1);
//                 }
//             },
//             Scope(ref lines) => {
//                 println!("{}Scope", buffer);
//                 for expr in lines {
//                     expr.print(buf + 1);
//                 }
//             }
//             Assignment(ref key, ref name, is_mut, ref val) => {
//                 println!("{}{} Assignment ({})", buffer, key, is_mut);
//                 name.print(buf + 1);
//                 val.print(buf + 1);
//             },
//             FnCall(ref fun, ref args) => {
//                 println!("{}Function Call", buffer);

//                 fun.print(buf + 1);
//                 for arg in args {
//                     arg.print(buf + 1);
//                 }
//             },
//             Operator(ref op, ref lhs, ref rhs) => {
//                 match *op {
//                     BindingType::Operator(ref op) => println!("{}Operator {}", buffer, op),
//                     _ => println!("{}Invalid operator construct", buffer),
//                 }
//                 lhs.print(buf + 1);
//                 rhs.print(buf + 1);
//             }
//         }
//     }
// }