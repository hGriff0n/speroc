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

	Future::Future(bool f, Location loc) : ValExpr{ loc }, generated{ f } {}
	Visitor& Future::visit(Visitor& v) {
		//v.acceptFuture(*this);
		return v;
	}
	DEF_PRINTER(Future) {
		return s << std::string(buf, ' ') << context << "ast.FutureValue (gen=" << generated << ')';
	}

	Tuple::Tuple(std::deque<ptr<ValExpr>> vals, Location loc) : Sequence{ std::move(vals), loc } {}
	Visitor& Tuple::visit(Visitor& v) {
		//v.acceptTuple(*this);
		return v;
	}
	DEF_PRINTER(Tuple) {
		s << std::string(buf, ' ') << context << "ast.Tuple" << elems.size() << " (";
		for (auto&& e : elems) {
			e->prettyPrint(s << '\n', buf + 2) << ",";
		}

		if (elems.size()) {
			s << '\n' << std::string(buf, ' ');
		}
		return s << ')';
	}

	Array::Array(std::deque<ptr<ValExpr>> vals, Location loc) : Sequence{ std::move(vals), loc } {}
	Visitor& Array::visit(Visitor& v) {
		//v.acceptArray(*this);
		return v;
	}
	DEF_PRINTER(Array) {
		s << std::string(buf, ' ') << context << "ast.Array" << elems.size() << " [type={}] [";
		for (auto&& e : elems) {
			e->prettyPrint(s << '\n', buf + 2) << ",";
		}

		if (elems.size()) {
			s << '\n' << std::string(buf, ' ');
		}
		return s << ']';
	}

	Block::Block(std::deque<ptr<Statement>> vals, Location loc) : Sequence{ std::move(vals), loc } {}
	Visitor& Block::visit(Visitor& v) {
		//v.acceptBlock(*this);
		return v;
	}
	DEF_PRINTER(Block) {
		s << std::string(buf, ' ') << context << "ast.Block (size=" << elems.size() << ") {";
		for (auto&& e : elems) {
			e->prettyPrint(s << '\n', buf + 2);
		}

		if (elems.size()) {
			s << '\n' << std::string(buf, ' ');
		}
		return s << '}';
	}

	Function::Function(ptr<Tuple> args, ptr<ValExpr> body, Location loc)
		: ValExpr{ loc }, args{ std::move(args) }, body{ std::move(body) } {}
	Visitor& Function::visit(Visitor& v) {
		//v.acceptFunction(*this);
		return v;
	}
	DEF_PRINTER(Function) {
		s << std::string(buf, ' ') << context << "ast.Function";
		ValExpr::prettyPrint(s, buf);

		if (args) {
			args->prettyPrint(s << '\n', buf + 2, "args=");
		}
		return body->prettyPrint(s << '\n', buf + 2, "expr=");
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

	Pattern::Pattern(Location loc) : Ast{ loc } {}
	Visitor& Pattern::visit(Visitor& v) {
		//v.acceptPattern(*this);
		return v;
	}
	DEF_PRINTER(Pattern) {
		return s << std::string(buf, ' ') << context << "ast.AnyPattern (_)";
	}

	TuplePattern::TuplePattern(std::deque<ptr<Pattern>> ps, Location loc)
		: Sequence{ std::move(ps), loc } {}
	Visitor& TuplePattern::visit(Visitor& v) {
		//v.acceptPTuple(*this);
		return v;
	}
	DEF_PRINTER(TuplePattern) {
		s << std::string(buf, ' ') << context << "ast.TuplePattern" << elems.size() << " (capture=" << cap._to_string() << ") (\n";
		for (auto&& p : elems) {
			p->prettyPrint(s, buf + 2) << '\n';
		}
		return s << std::string(buf, ' ') << ')';
	}

	VarPattern::VarPattern(ptr<QualifiedBinding> n, Location loc) : Pattern{ loc }, name{ std::move(n) } {}
	Visitor& VarPattern::visit(Visitor& v) {
		//v.acceptPNamed(*this);
		return v;
	}
	DEF_PRINTER(VarPattern) {
		s << std::string(buf, ' ') << context << "ast.VarPattern (capture=" << cap._to_string() << ')';
		name->prettyPrint(s, 1);
		/*if (type) {
			type->prettyPrint(s << '\n', buf + 2, "type=");
		}*/

		return s;
	}

	AdtPattern::AdtPattern(ptr<QualifiedBinding> n, ptr<TuplePattern> pat, Location loc)
		: VarPattern{ std::move(n), loc }, args{ std::move(pat) } {}
	Visitor& AdtPattern::visit(Visitor& v) {
		//v.acceptPAdt(*this);
		return v;
	}
	DEF_PRINTER(AdtPattern) {
		s << std::string(buf, ' ') << context << "ast.AdtPattern";
		name->prettyPrint(s, 1, "(type=") << ')';
		if (args) {
			args->prettyPrint(s << '\n', buf + 2);
		}
		return s;
	}

	ValPattern::ValPattern(ptr<ValExpr> v, Location loc) : Pattern{ loc }, val{ std::move(v) } {}
	Visitor& ValPattern::visit(Visitor& v) {
		//v.acceptPVal(*this);
		return v;
	}
	DEF_PRINTER(ValPattern) {
		s << std::string(buf, ' ') << context << "ast.PVal";
		return val->prettyPrint(s, 1, "(val=") << ')';
	}


	//
	// TYPES
	//
	SourceType::SourceType(ptr<QualifiedBinding> b, Location loc)
		: Type{ loc }, name{ std::move(b) }, _ptr{ PtrStyling::NA } {}
	Visitor& SourceType::visit(Visitor& v) {
		//v.acceptSourceType(*this);
		return v;
	}
	DEF_PRINTER(SourceType) {
		s << std::string(buf, ' ') << context << "ast.SourceType \"";
		return name->prettyPrint(s, 0) << "\" (ptr=" << _ptr._to_string() << ")";
	}

	GenericType::GenericType(ptr<QualifiedBinding> b, ptr<Array> a, Location loc)
		: SourceType{ std::move(b), loc }, inst{ std::move(a) } {}
	Visitor& GenericType::visit(Visitor& v) {
		//v.acceptGenericType(*this);
		return v;
	}
	DEF_PRINTER(GenericType) {
		s << std::string(buf, ' ') << context << "ast.GenericType \"";
		name->prettyPrint(s, 0) << "\" (ptr=" << _ptr._to_string() << ")";

		if (inst && inst->elems.size()) {
			s << " [\n";
			for (auto&& e : inst->elems) {
				e->prettyPrint(s, buf + 2) << '\n';
			}
			s << std::string(buf, ' ') << "]";
		}

		return s;
	}

	TupleType::TupleType(std::deque<ptr<Type>> ts, Location loc) : Sequence{ std::move(ts), loc } {}
	Visitor& TupleType::visit(Visitor& v) {
		//v.acceptTupleType(*this);
		return v;
	}
	DEF_PRINTER(TupleType) {
		s << std::string(buf, ' ') << context << "ast.TupleType" << elems.size() << "(";

		for (auto&& t : elems) {
			t->prettyPrint(s << '\n', buf + 2);
		}

		return s << '\n' << std::string(buf, ' ') << ')';
	}

	FunctionType::FunctionType(ptr<TupleType> as, ptr<Type> ret, Location loc)
		: Type{ loc }, args{ std::move(as) }, ret{ std::move(ret) } {}
	Visitor& FunctionType::visit(Visitor& v) {
		//v.acceptFunctionType(*this);
		return v;
	}
	DEF_PRINTER(FunctionType) {
		s << std::string(buf, ' ') << context << "ast.FunctionType";

		args->prettyPrint(s << '\n', buf + 2, "args=");
		return ret->prettyPrint(s << '\n', buf + 2, "ret=");
	}

	AndType::AndType(ptr<Type> typs, Location loc)
		: Sequence{ MK_DEQUE<Type>(std::move(typs)), loc } {}
	Visitor& AndType::visit(Visitor& v) {
		//v.acceptAndType(*this);
		return v;
	}
	DEF_PRINTER(AndType) {
		s << std::string(buf, ' ') << context << "ast.AndType";

		for (auto&& t : elems) {
			t->prettyPrint(s << '\n', buf + 2);
		}

		return s;
	}

	OrType::OrType(ptr<Type> typs, Location loc)
		: Sequence{ MK_DEQUE<Type>(std::move(typs)), loc } {}
	Visitor& OrType::visit(Visitor& v) {
		//v.acceptOrType(*this);
		return v;
	}
	DEF_PRINTER(OrType) {
		s << std::string(buf, ' ') << context << "ast.OrType";

		for (auto&& t : elems) {
			t->prettyPrint(s << '\n', buf + 2);
		}

		return s;
	}


	//
	// DECORATIONS, ANNOTATION, GENERIC
	//

	Annotation::Annotation(ptr<BasicBinding> n, ptr<Tuple> t, Location loc)
		: Ast{ loc }, name{ std::move(n) }, args{ std::move(t) } {}
	Visitor& Annotation::visit(Visitor& v) {
		//v.acceptAnnotation(*this);
		return v;
	}
	DEF_PRINTER(Annotation) {
		s << std::string(buf, ' ') << context << "ast.Annotation name=";
		name->prettyPrint(s, 0);

		if (args) {
			args->prettyPrint(s << '\n', buf + 2, "args=");
		}
		return s;
	}

	LocalAnnotation::LocalAnnotation(ptr<BasicBinding> n, ptr<Tuple> t, Location loc)
		: Annotation{ std::move(n), std::move(t), loc } {}
	Visitor& LocalAnnotation::visit(Visitor& v) {
		//v.acceptLocalAnnotation(*this);
		return v;
	}
	DEF_PRINTER(LocalAnnotation) {
		s << std::string(buf, ' ') << context << "ast.LocalAnnotation name=";
		name->prettyPrint(s, 0);

		if (args) {
			args->prettyPrint(s << '\n', buf + 2, "args=");
		}
		return s;
	}

	Adt::Adt(ptr<BasicBinding> n, ptr<TupleType> t, Location loc)
		: Ast{ loc }, name{ std::move(n) }, args{ std::move(t) } {}
	Visitor& Adt::visit(Visitor& v) {
		//v.acceptAdt(*this);
		return v;
	}
	DEF_PRINTER(Adt) {
		s << std::string(buf, ' ') << context << "ast.Adt (name=";
		name->prettyPrint(s, 0) << ")";

		if (args) {
			args->prettyPrint(s << '\n', buf + 2, "args=");
		}
		return s;
	}


	//
	// CONTROL
	//

	namespace {}


	//
	// STMTS
	//

	namespace {}

}