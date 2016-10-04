use std::fmt;
use std::iter;

#[derive(Debug)]
pub enum LiteralType {
    Num(String, Option<String>),
    Byte(String, bool),
    Bool(bool),
    Str(String),
    Char(char),
}

#[derive(Debug)]
pub enum BindingType {
    Variable(String),
    Type(String),
    Operator(String),
    Tuple(Vec<BindingType>),
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
    Assignment(Keyword, BindingType, bool, Box<ExprType>),
    Literal(LiteralType),
    Scope(Vec<ExprType>),
    Tuple(Vec<ExprType>),
    Binary(BindingType, Box<ExprType>, Box<ExprType>),
    FnCall(BindingType, Option<Box<ExprType>>, Option<Box<ExprType>>),
    ModDecl(Vec<BindingType>),
}

static mut buf_size: usize = 0;

fn mk_buffer(size: usize) -> String {
    iter::repeat(" ").take(size).collect()
}

impl fmt::Display for LiteralType {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        unsafe {
            let buff = mk_buffer(buf_size);

            match *self {
                LiteralType::Num(ref int_part, ref float_part) => {
                    match *float_part {
                        Some(ref flt) => {
                            write!(f, "{}Float: {}.{}", buff, int_part, flt)
                        },
                        None => {
                            write!(f, "{}Int: {}", buff, int_part)
                        }
                    }
                },
                LiteralType::Byte(ref string, is_hex) => {
                    write!(f, "{}Byte: {}", buff, string)
                },
                LiteralType::Bool(val) => {
                    write!(f, "{}Bool: {}", buff, val)
                },
                LiteralType::Str(ref string) => {
                    write!(f, "{}String: {}", buff, string)
                },
                LiteralType::Char(ref ch) => {
                    write!(f, "{}Char: {}", buff, ch)
                }
            }
        }
    }
}

impl fmt::Display for BindingType {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        unsafe {
            let buf = mk_buffer(buf_size);

            match *self {
                BindingType::Variable(ref var) => write!(f, "{}Variable: {}", buf, var),
                BindingType::Type(ref typ) => write!(f, "{}Type: {}", buf, typ),
                BindingType::Operator(ref op) => write!(f, "{}Op: {}", buf, op),
                BindingType::Tuple(ref binds) => {
                    let o_size = buf_size;
                    buf_size += 1;

                    write!(f, "{}TupleBinding[{}]", buf, binds.len());

                    for piece in binds {
                        write!(f, "\n{}", piece);
                    }

                    buf_size -= 1;
                    write!(f, "")
                }
            }
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
        unsafe {
            let buf = mk_buffer(buf_size);
            let o_size = buf_size;

            match *self {
                ExprType::Assignment(ref key, ref bind, is_mut, ref val) => {
                    writeln!(f, "{}Binding ({}, {})", buf, key, is_mut);

                    buf_size += 1;
                    writeln!(f, "{}", bind);
                    write!(f, "{}", val);
                },
                ExprType::Literal(ref literal) => {
                    write!(f, "{}", literal);
                },
                ExprType::Scope(ref exprs) => {
                    write!(f, "{}Scope", buf);
                    
                    buf_size += 1;
                    for expr in exprs {
                        write!(f, "\n{}", expr);
                    }
                },
                ExprType::Tuple(ref exprs) => {
                    write!(f, "{}Tuple[{}]", buf, exprs.len());

                    buf_size += 1;
                    for expr in exprs {
                        write!(f, "\n{}", expr);
                    }
                },
                ExprType::Binary(ref bind, ref lhs, ref rhs) => {
                    writeln!(f, "{}Binary Operator {}", buf, bind);
                    
                    buf_size += 1;
                    writeln!(f, "{}", lhs);
                    write!(f, "{}", rhs);
                },
                ExprType::FnCall(ref fun, ref caller, ref args) => {
                    write!(f, "{}A function call", buf);
                },
                ExprType::ModDecl(ref module) => {
                    write!(f, "{}Module Declaration", buf);

                    buf_size += 1;
                    for piece in module {
                        write!(f, "\n{}", piece);
                    }
                }
            }

            buf_size = o_size;
            write!(f, "")
        }
    }
}