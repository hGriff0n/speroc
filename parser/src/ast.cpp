#include "ast/decor.h"
#include "ast/atoms.h"
#include "ast/names.h"

namespace spero::compiler::ast {
	
	/*
	 * ast::Ast
	 */
	Ast::Ast() {}
	OutStream& Ast::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.Ast";
	}


	/*
	 * ast::Token
	 */
	
	OutStream& Token::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Token ";

		if (std::holds_alternative<KeywordType>(value))
			return s << std::get<KeywordType>(value)._to_string();
		else if (std::holds_alternative<PtrStyling>(value))
			return s << std::get<PtrStyling>(value)._to_string();
		else if (std::holds_alternative<VarianceType>(value))
			return s << std::get<VarianceType>(value)._to_string();
		else if (std::holds_alternative<RelationType>(value))
			return s << std::get<RelationType>(value)._to_string();
		else if (std::holds_alternative<VisibilityType>(value))
			return s << std::get<VisibilityType>(value)._to_string();
		else if (std::holds_alternative<BindingType>(value))
			return s << std::get<BindingType>(value)._to_string();
		else if (std::holds_alternative<UnaryType>(value))
			return s << std::get<UnaryType>(value)._to_string();
		else
			return s << "err";
	}


	/*
	 * ast::Stmt
	 */
	Stmt::Stmt() {}
	OutStream& Stmt::prettyPrint(OutStream& s, size_t buf, std::string context) {
		for (auto&& a : annots)
			a->prettyPrint(s << '\n', buf + 2);

		return s;
	}


	/*
	 * ast::ValExpr
	 */
	ValExpr::ValExpr() {}
	OutStream& ValExpr::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << "mut=" << is_mut << ')';

		if (type)
			type->prettyPrint(s << '\n', buf + 2, "type=");

		return Stmt::prettyPrint(s, buf);
	}


	/*
	 * ast::Error
	 */
	/*Error::Error(std::string msg) : msg{ std::move(msg) } {}
	OutStream& Error::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.Error " << msg;
	}*/

}
