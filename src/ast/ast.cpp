#include "ast/ast.h"
#include "util/utils.h"
#include <iterator>

namespace spero::compiler::ast {

	// Literals
	Byte::Byte(const std::string& num, int base) : val{ std::stoul(num, nullptr, base) } {}
	Int::Int(const std::string& num) : val{ std::stoi(num) } {}
	Float::Float(const std::string& num) : val{ std::stof(num) } {}
	String::String(const std::string& str) : val{ str } {}
	Char::Char(char c) : val{ c } {}
	Bool::Bool(bool b) : val{ b } {}
	
	// Bindings
	//BasicName::BasicName(const std::string& name) : name{ name } {}
	//Operator::Operator(const std::string& name) : op{ name } {}

}
