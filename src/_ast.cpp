#include "parser/_visitor.h"

namespace spero::compiler::ast {
	// Only defined as `std::deque` has no `initializer_list` constructor
	template<class T>
	std::deque<ptr<T>> MK_DEQUE(ptr<T> e) { std::deque<ptr<T>> ret; ret.emplace_back(std::move(e)); return std::move(ret); }
	template<class T>
	std::deque<ptr<T>> MK_DEQUE(ptr<T> e1, ptr<T> e2) { std::deque<ptr<T>> ret; ret.emplace_back(std::move(e1)); ret.emplace_back(std::move(e2)); return std::move(ret); }

	// Helper define to reduce typing
#define DEF_PRINTER(typ) std::ostream& typ::prettyPrint(std::ostream& s, size_t buf, std::string context)


	//
	// BASE NODES: AST, VALEXPR, STMT
	// 

	Ast::Ast(Location loc) : loc{ loc } {}
	Visitor& Ast::visit(Visitor& v) {
		//v.accept(*this);
		return v;
	}
	DEF_PRINTER(Ast) {
		return s << std::string(buf, ' ') << context << "ast.Ast";
	}

	Visitor& Token::visit(Visitor& v) {
		//v.acceptToken(*this);
		return v;
	}
	DEF_PRINTER(Token) {
		s << std::string(buf, ' ') << context << "ast.Token ";

		if (std::holds_alternative<KeywordType>(value)) {
			return s << std::get<KeywordType>(value)._to_string();
		} else if (std::holds_alternative<PtrStyling>(value)) {
			return s << std::get<PtrStyling>(value)._to_string();
		} else if (std::holds_alternative<VarianceType>(value)) {
			return s << std::get<VarianceType>(value)._to_string();
		} else if (std::holds_alternative<RelationType>(value)) {
			return s << std::get<RelationType>(value)._to_string();
		} else if (std::holds_alternative<VisibilityType>(value)) {
			return s << std::get<VisibilityType>(value)._to_string();
		} else if (std::holds_alternative<BindingType>(value)) {
			return s << std::get<BindingType>(value)._to_string();
		} else if (std::holds_alternative<UnaryType>(value)) {
			return s << std::get<UnaryType>(value)._to_string();
		} else {
			return s << "err";
		}
	}

	Type::Type(Location loc) : Ast{ loc } {}
	Visitor& Type::visit(Visitor& v) {
		//v.acceptType(*this);
		return v;
	}

	Statement::Statement(Location loc) : Ast{ loc } {}
	Visitor& Statement::visit(Visitor& v) {
		//v.acceptStmt(*this);
		return v;
	}
	DEF_PRINTER(Statement) {
		for (auto&& a : annots) {
			a->prettyPrint(s << '\n', buf + 2);
		}

		return s;
	}

	ValExpr::ValExpr(Location loc) : Statement{ loc } {}
	Visitor& ValExpr::visit(Visitor& v) {
		//v.acceptValExpr(*this);
		return v;
	}
	DEF_PRINTER(ValExpr) {
		s << "mut=" << is_mut << ')';

		if (type) {
			type->prettyPrint(s << '\n', buf + 2, "type=");
		}

		return Statement::prettyPrint(s, buf);
	}


	//
	// LITERALS, ATOMS
	//

	Literal::Literal(Location loc) : ValExpr{ loc } {}

	Bool::Bool(bool b, Location loc) : Literal{ loc }, val{ b } {}
	Visitor& Bool::visit(Visitor& v) {
		//v.acceptBool(*this);
		return v;
	}
	DEF_PRINTER(Bool) {
		s << std::string(buf, ' ') << context << "ast.Bool (val=" << (val ? "true, " : "false, ");

		return ValExpr::prettyPrint(s, buf);
	}

	Byte::Byte(std::string num, int base, Location loc) : Literal{ loc }, val{ std::stoul(num, nullptr, base) } {}
	Visitor& Byte::visit(Visitor& v) {
		//v.acceptByte(*this);
		return v;
	}
	DEF_PRINTER(Byte) {
		s << std::string(buf, ' ') << context << "ast.Byte (val=" << val << ", ";

		return ValExpr::prettyPrint(s, buf);
	}

	Float::Float(std::string num, Location loc) : Literal{ loc }, val{ std::stof(num) } {}
	Visitor& Float::visit(Visitor& v) {
		//v.acceptFloat(*this);
		return v;
	}
	DEF_PRINTER(Float) {
		s << std::string(buf, ' ') << context << "ast.Float (val=" << val << ", ";

		return ValExpr::prettyPrint(s, buf);
	}

	Int::Int(std::string num, Location loc) : Literal{ loc }, val{ std::stol(num) } {}
	Visitor& Int::visit(Visitor& v) {
		//v.acceptInt(*this);
		return v;
	}
	DEF_PRINTER(Int) {
		s << std::string(buf, ' ') << context << "ast.Int (val=" << val << ", ";

		return ValExpr::prettyPrint(s, buf);
	}

	Char::Char(char c, Location loc) : Literal{ loc }, val{ c } {}
	Visitor& Char::visit(Visitor& v) {
		//v.acceptChar(*this);
		return v;
	}
	DEF_PRINTER(Char) {
		s << std::string(buf, ' ') << context << "ast.Char (val='" << val << "', ";

		return ValExpr::prettyPrint(s, buf);
	}

	String::String(std::string v, Location loc) : ValExpr{ loc }, val{ v } {}
	Visitor& String::visit(Visitor& v) {
		//v.acceptString(*this);
		return v;
	}
	DEF_PRINTER(String) {
		s << std::string(buf, ' ') << context << "ast.String (val=\"" << val << "\", ";

		return ValExpr::prettyPrint(s, buf);
	}


	// 
	// NAMES, BINDINGS, PATTERNS
	//

	BasicBinding::BasicBinding(std::string n, BindingType t, Location loc)
		: Ast{ loc }, name{ n }, type{ t } {}
	Visitor& BasicBinding::visit(Visitor& v) {
		//v.acceptBasicBinding(*this);
		return v;
	}
	DEF_PRINTER(BasicBinding) {
		return s << std::string(buf, ' ') << context << "ast.BasicBinding (var=" << name << ", type=" << type._to_string() << ")";
	}
	std::string BasicBinding::toString() {
		return name;
	}

	QualifiedBinding::QualifiedBinding(ptr<BasicBinding> b, Location loc) : Sequence{ MK_DEQUE(std::move(b)), loc } {}
	QualifiedBinding::QualifiedBinding(std::deque<ptr<BasicBinding>> ps, Location loc) : Sequence{ std::move(ps), loc } {}
	Visitor& QualifiedBinding::visit(Visitor& v) {
		//v.acceptQualifiedBinding(*this);
		return v;
	}
	DEF_PRINTER(QualifiedBinding) {
		return s << std::string(buf, ' ') << context << "ast.QualifiedBinding " << toString();
	}
	std::string QualifiedBinding::toString() {
		std::string result = elems.front()->toString();

		for (auto p = std::begin(elems) + 1; p != std::end(elems); ++p) {
			result += ":" + p->get()->toString();
		}

		return result;
	}


	//
	// TYPES
	//

	namespace {}


	//
	// DECORATIONS, ANNOTATION, GENERIC
	//

	/*Annotation::Annotation(ptr<BasicBinding> n, ptr<Tuple> t, Location loc)
		: Ast{ loc }, name{ std::move(n) }, args{ std::move(t) } {}*/
	Annotation::Annotation(ptr<BasicBinding> n, Location loc)
		: Ast{ loc }, name{ std::move(n) } {}
	Visitor& Annotation::visit(Visitor& v) {
		//v.acceptAnnotation(*this);
		return v;
	}
	DEF_PRINTER(Annotation) {
		s << std::string(buf, ' ') << context << "ast.Annotation name=";
		name->prettyPrint(s, 0);

		/*if (args) {
			args->prettyPrint(s << '\n', buf + 2, "args=");
		}*/
		return s;
	}

	/*LocalAnnotation::LocalAnnotation(ptr<BasicBinding> n, ptr<Tuple> t, Location loc)
		: Annotation{ std::move(n), std::move(t), loc } {}*/
	LocalAnnotation::LocalAnnotation(ptr<BasicBinding> n, Location loc)
		: Annotation{ std::move(n), loc } {}
	Visitor& LocalAnnotation::visit(Visitor& v) {
		//v.acceptLocalAnnotation(*this);
		return v;
	}
	DEF_PRINTER(LocalAnnotation) {
		s << std::string(buf, ' ') << context << "ast.LocalAnnotation name=";
		name->prettyPrint(s, 0);

		/*if (args) {
			args->prettyPrint(s << '\n', buf + 2, "args=");
		}*/
		return s;
	}


	//
	// CONTROL
	//

	namespace {}


	//
	// STMTS
	//

}