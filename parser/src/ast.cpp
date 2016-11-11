#include "ast.h"
//#include "util/utils.h"

#include <iterator>
#include <sstream>

//using namespace spero::util;

namespace spero::compiler::ast {
	/*std::string ValExpr::pretty_fmt(int depth) {
		std::stringstream s;
		s << std::string(depth, ' ') << "Printing an astnode";
		return s.str();
	}*/

	//
	// Byte
	//
	Byte::Byte(const std::string& num, int base) : val{ std::stoul(num, nullptr, base) } {}
	std::string Byte::pretty_fmt(int depth) {
		std::stringstream s;
		s << std::string(depth, ' ') << "Byte: " << val;
		return s.str();
	}

	//
	// Int
	//
	Int::Int(const std::string& num) : val{ std::stoi(num) } {}
	std::string Int::pretty_fmt(int depth) {
		std::stringstream s;
		s << std::string(depth, ' ') << "Int: " << val;
		return s.str();
	}

	//
	// Float
	//
	Float::Float(const std::string& num) : val{ std::stof(num) } {}
	std::string Float::pretty_fmt(int depth) {
		std::stringstream s;
		s << std::string(depth, ' ') << "Float: " << val;
		return s.str();
	}

	//
	// String
	//
	String::String(const std::string& str) : val{ str } {}
	std::string String::pretty_fmt(int depth) {
		return std::string(depth, ' ') + "String: " + val;
	};

	//
	// Char
	//
	Char::Char(char c) : val{ c } {}
	std::string Char::pretty_fmt(int depth) {
		return std::string(depth, ' ') + "Char: " + val;
	}

	//
	// Bool
	//
	Bool::Bool(bool b) : val{ b } {}
	std::string Bool::pretty_fmt(int depth) {
		return std::string(depth, ' ') + "Bool: " + (val ? "true" : "false");
	}
}
