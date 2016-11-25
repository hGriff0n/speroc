//#include "ast/literals.h"
//#include "ast/atoms.h"
#include "ast/all.h"

#include <string>

namespace spero::compiler::ast {

	/*
	 * ast::Bool
	 */
	Bool::Bool(bool b) : val{ b } {}
	OutStream Bool::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::Byte
	 */
	Byte::Byte(std::string num, int base) : val{ std::stoul(std::move(num), nullptr, base) } {}
	OutStream Byte::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::Float
	 */
	Float::Float(std::string num) : val{ std::stof(std::move(num)) } {}
	OutStream Float::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::Int
	 */
	Int::Int(std::string num) : val{ std::stoi(std::move(num)) } {}
	OutStream Int::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::String
	 */
	String::String(std::string v) : val{ std::move(v) } {}
	OutStream String::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::Char
	 */
	Char::Char(char c) : val{ c } {}
	OutStream Char::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::FnBody
	 */
	FnBody::FnBody(ptr<ValExpr> b, bool f) : forward{ f }, body{ std::move(b) } {}
	OutStream FnBody::prettyPrint(OutStream s, size_t buf) {
		return s;
	}

}