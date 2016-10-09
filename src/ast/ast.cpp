#include "ast\ast.h"

namespace spero::compiler::ast {

	Byte::Byte(const std::string& num, int base) : val{ std::stoi(num, nullptr, base) } {}

	Int::Int(const std::string& num) : val{ std::stoi(num) } {}

	Float::Float(const std::string& num) : val{ std::stof(num) } {}

	String::String(const std::string& str) : val{ str } {}

	Char::Char(char c) : val{ c } {}

}