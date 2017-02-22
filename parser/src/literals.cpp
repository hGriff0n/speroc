#include "ast/literals.h"
#include "ast/atoms.h"
#include "ast/decor.h"
#include "ast/names.h"

namespace spero::compiler::ast {

	/*
	 * ast::Bool
	 */
	Bool::Bool(bool b, Ast::Location loc) : ValExpr{ loc }, val{ b } {}
	OutStream& Bool::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Bool (val=" << (val ? "true, " : "false, ");

		return ValExpr::prettyPrint(s, buf);
	}


	/*
	 * ast::Byte
	 */
	Byte::Byte(std::string num, int base, Ast::Location loc)
		: ValExpr{ loc }, val { std::stoul(std::move(num), nullptr, base) } {}
	OutStream& Byte::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Byte (val=" << val << ", ";

		return ValExpr::prettyPrint(s, buf);
	}


	/*
	 * ast::Float
	 */
	Float::Float(std::string num, Ast::Location loc) : ValExpr{ loc }, val{ std::stof(std::move(num)) } {}
	OutStream& Float::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Float (val=" << val << ", ";

		return ValExpr::prettyPrint(s, buf);
	}


	/*
	 * ast::Int
	 */
	Int::Int(std::string num, Ast::Location loc) : ValExpr{ loc }, val{ std::stoi(std::move(num)) } {}
	OutStream& Int::prettyPrint(OutStream& s, size_t buf, std::string context) {
		// Print out Int specific values
		s << std::string(buf, ' ') << context << "ast.Int (val=" << val << ", ";

		return ValExpr::prettyPrint(s, buf);
	}
	OutStream& Int::assemblyCode(OutStream& s) {
		return s << "\tmov $" << val << ", %eax\n";
	}


	/*
	 * ast::String
	 */
	String::String(std::string v, Ast::Location loc) : ValExpr{ loc }, val{ std::move(v) } {}
	OutStream& String::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.String (val=\"" << val << "\", ";

		return ValExpr::prettyPrint(s, buf);
	}


	/*
	 * ast::Char
	 */
	Char::Char(char c, Ast::Location loc) : ValExpr{ loc }, val{ c } {}
	OutStream& Char::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Char (val='" << val << ", ";

		return ValExpr::prettyPrint(s, buf);
	}


	/*
	 * ast::FnBody
	 */
	FnBody::FnBody(ptr<ValExpr> b, bool f, Ast::Location loc) : ValExpr{ loc }, forward{ f }, body{ std::move(b) } {}
	OutStream& FnBody::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.FnBody (fwd=" << (forward ? "true, " : "false, ");
		ValExpr::prettyPrint(s, buf);

		if (ret) ret->prettyPrint(s << '\n', buf + 2, "returns=");
		if (args) args->prettyPrint(s << '\n', buf + 2, "args=");
		return body->prettyPrint(s << '\n', buf + 2, "expr=");
	}
	OutStream& FnBody::assemblyCode(OutStream& out) {
		out << "\t.p2align 4, 0x90\n"
			<< "\t.globl _main\n"
			<< "\t.def _main; .scl 2; .type 32; .endef\n"
			<< "_main:\n"
			<< "LFB0:\n"
			<< "\tpush %ebp\n"
			<< "\tmov %esp, %ebp\n"
			<< "\tpush %eax\n";

		// Print the int (what does the fn have?)
		body->assemblyCode(out);

		// Print function tail/endlog
		return out << "\tleave\n"
			<< "\tret\n"
			<< "LFE0:\n"
			<< "\t.ident \"speroc: 0.0.1 (Windows 2016)\"";
	}

}