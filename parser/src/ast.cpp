#include "ast.h"
//#include "util/utils.h"

#include <iterator>

//using namespace spero::util;

namespace spero::compiler::ast {
	//
	// Parent Nodes
	//
	Ast::Ast() {}
	Token::Token(KeywordType t) : val{ t } {}
	Token::Token(PtrStyling t) : val{ t } {}
	Token::Token(VarianceType t) : val{ t } {}
	Token::Token(RelationType t) : val{ t } {}
	Token::Token(VisibilityType t) : val{ t } {}
	Token::Token(BindingType t) : val{ t } {}
	Token::Token(UnaryType t) : val{ t } {}
	Stmt::Stmt() {}
	ValExpr::ValExpr() {}


	//
	// Byte
	//
	Byte::Byte(const std::string& num, int base) : val{ std::stoul(num, nullptr, base) } {}

	//
	// Int
	//
	Int::Int(const std::string& num) : val{ std::stoi(num) } {}

	//
	// Float
	//
	Float::Float(const std::string& num) : val{ std::stof(num) } {}

	//
	// String
	//
	String::String(const std::string& str) : val{ str } {}

	//
	// Char
	//
	Char::Char(char c) : val{ c } {}

	//
	// Bool
	//
	Bool::Bool(bool b) : val{ b } {}
}
