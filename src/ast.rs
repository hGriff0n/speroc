use std::fmt;


#[derive(Debug)]
pub enum LiteralType {
    Num(String, Option<String>),
    Byte(String, bool),
    Bool(bool),
    Str(String),
    Char(char),
}

#[derive(Debug)]
pub enum AtomType {
    Literal(LiteralType),
}

#[derive(Debug)]
pub enum BindingType {
    Variable(String),
    Type(String),
    Operator(String),
}

// Here we see one of the biggest "problems" with Rust, the borrow checker
#[derive(Debug, Clone, Copy)]
pub enum Keyword {
    Def,
    Let,
    Static,
    Mut,
}

#[derive(Debug)]
pub enum ExprType {
}

impl fmt::Display for LiteralType {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match *self {
            LiteralType::Num(ref int_part, ref float_part) => {
                match *float_part {
                    Some(ref flt) => {
                        writeln!(f, "Float: {}.{}", int_part, flt)
                    },
                    None => {
                        writeln!(f, "Int: {}", int_part)
                    }
                }
            },
            LiteralType::Byte(ref string, is_hex) => {
                writeln!(f, "Byte: {}", string)
            },
            LiteralType::Bool(val) => {
                writeln!(f, "Bool: {}", val)
            },
            LiteralType::Str(ref string) => {
                writeln!(f, "String: {}", string)
            },
            LiteralType::Char(ref ch) => {
                writeln!(f, "Char: {}", ch)
            }
        }
    }
}

impl fmt::Display for BindingType {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match *self {
            BindingType::Variable(ref var) => write!(f, "Variable: {}", var),
            BindingType::Type(ref typ) => write!(f, "Type: {}", typ),
            BindingType::Operator(ref op) => write!(f, "Op: {}", op),
        }
    }
}

impl fmt::Display for Keyword {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match *self {
            Keyword::Def => write!(f, "def"),
            Keyword::Let => write!(f, "let"),
            Keyword::Static => write!(f, "static"),
            Keyword::Mut => write!(f, "mut"),
        }
    }
}

#[allow(unused_variables)]
impl fmt::Display for ExprType {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
                write!(f, "")
        }
}