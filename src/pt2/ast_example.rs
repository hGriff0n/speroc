
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
//#[derive(Debug)]
#[derive(Debug, Clone, Copy)]
pub enum Keyword {
    Def,
    Let,
    Static,
    Mut,
}

#[derive(Debug)]
pub enum ExprType {
    Literal(AtomType),
    Tuple(Vec<ExprType>),
    FnCall(BindingType, Option<ExprType>, Option<ExprType>),
    // FnCall(BindingType, Option<Box<ExprType>>, Option<Box<ExprType>>),
}