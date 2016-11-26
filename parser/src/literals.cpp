#include "ast/literals.h"
#include "ast/atoms.h"
#include "ast/decor.h"
#include "ast/names.h"

namespace spero::compiler::ast {

	/*
	 * ast::Bool
	 */
	Bool::Bool(bool b) : val{ b } {}
	OutStream& Bool::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Bool (val=" << (val ? "true, " : "false, ");

		return ValExpr::prettyPrint(s, buf);
	}


	/*
	 * ast::Byte
	 */
	Byte::Byte(std::string num, int base) : val{ std::stoul(std::move(num), nullptr, base) } {}
	OutStream& Byte::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Byte (val=" << val << ", ";

		return ValExpr::prettyPrint(s, buf);
	}


	/*
	 * ast::Float
	 */
	Float::Float(std::string num) : val{ std::stof(std::move(num)) } {}
	OutStream& Float::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Float (val=" << val << ", ";

		return ValExpr::prettyPrint(s, buf);
	}


	/*
	 * ast::Int
	 */
	Int::Int(std::string num) : val{ std::stoi(std::move(num)) } {}
	OutStream& Int::prettyPrint(OutStream& s, size_t buf, std::string context) {
		// Print out Int specific values
		s << std::string(buf, ' ') << context << "ast.Int (val=" << val << ", ";

		return ValExpr::prettyPrint(s, buf);
	}


	/*
	 * ast::String
	 */
	String::String(std::string v) : val{ std::move(v) } {}
	OutStream& String::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.String (val=\"" << val << "\", ";

		return ValExpr::prettyPrint(s, buf);
	}


	/*
	 * ast::Char
	 */
	Char::Char(char c) : val{ c } {}
	OutStream& Char::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Char (val='" << val << ", ";

		return ValExpr::prettyPrint(s, buf);
	}


	/*
	 * ast::FnBody
	 */
	FnBody::FnBody(ptr<ValExpr> b, bool f) : forward{ f }, body{ std::move(b) } {}
	OutStream& FnBody::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.FnBody (fwd=" << (forward ? "true, " : "false, ");
		ValExpr::prettyPrint(s, buf);

		if (ret) ret->prettyPrint(s << '\n', buf + 2, "returns=");
		if (args) args->prettyPrint(s << '\n', buf + 2, "args=");
		return body->prettyPrint(s << '\n', buf + 2, "expr=");
	}

}