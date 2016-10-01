
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

// Had to add in Clone to get it to compile
// Side note: rust's macros make debugging really hard
#[derive(Debug, Clone)]
pub enum Keyword {
    Def,
    Let,
    Static,
}

#[derive(Debug)]
pub enum ExprType {
    Assignment(Keyword, BindingType, AtomType),
}