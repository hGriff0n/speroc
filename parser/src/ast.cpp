//#include "ast/ast.h"
//#include "ast/decor.h"
//#include "ast/types.h"
#include "ast/all.h"

namespace spero::compiler::ast {
	
	/*
	 * ast::Ast
	 */
	Ast::Ast() {}
	OutStream Ast::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::Token
	 */
	Token::Token(KeywordType t) : value{ t } {}
	Token::Token(PtrStyling t) : value{ t } {}
	Token::Token(VarianceType t) : value{ t } {}
	Token::Token(RelationType t) : value{ t } {}
	Token::Token(VisibilityType t) : value{ t } {}
	Token::Token(BindingType t) : value{ t } {}
	Token::Token(UnaryType t) : value{ t } {}
	OutStream Token::prettyPrint(OutStream s, size_t buf) {
		return s;
		/*if (std::holds_alternative<KeywordType>(val))
			return std::get<KeywordType>(val)._to_string();
		else if (std::holds_alternative<PtrStyling>(val))
			return std::get<PtrStyling>(val)._to_string();
		else if (std::holds_alternative<VarianceType>(val))
			return std::get<VarianceType>(val)._to_string();
		else if (std::holds_alternative<RelationType>(val))
			return std::get<RelationType>(val)._to_string();
		else if (std::holds_alternative<VisibilityType>(val))
			return std::get<VisibilityType>(val)._to_string();
		else if (std::holds_alternative<BindingType>(val))
			return std::get<BindingType>(val)._to_string();
		else if (std::holds_alternative<UnaryType>(val))
			return std::get<UnaryType>(val)._to_string();
		else
			return "Empty";*/
	}


	/*
	 * ast::Stmt
	 */
	Stmt::Stmt() {}
	OutStream Stmt::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::ValExpr
	 */
	ValExpr::ValExpr() {}
	OutStream ValExpr::prettyPrint(OutStream s, size_t buf) {
		return s;
	}

}
