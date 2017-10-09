#include "parser/visitor.h"

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
	void Ast::accept(Visitor& v) {
		v.visit(*this);
	}
	DEF_PRINTER(Ast) {
		return s << std::string(buf, ' ') << context << "ast.Ast";
	}

	void Token::accept(Visitor& v) {
		v.visitToken(*this);
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
		} else {
			return s << "err";
		}
	}

	Type::Type(Location loc) : Ast{ loc } {}
	void Type::accept(Visitor& v) {
		v.visitType(*this);
	}

	Statement::Statement(Location loc) : Ast{ loc } {}
	void Statement::accept(Visitor& v) {
		v.visitStatement(*this);
	}
	DEF_PRINTER(Statement) {
		for (auto&& a : annots) {
			a->prettyPrint(s << '\n', buf + 2);
		}

		return s;
	}

	ValExpr::ValExpr(Location loc) : Statement{ loc } {}
	void ValExpr::accept(Visitor& v) {
		v.visitValExpr(*this);
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
	void Bool::accept(Visitor& v) {
		v.visitBool(*this);
	}
	DEF_PRINTER(Bool) {
		s << std::string(buf, ' ') << context << "ast.Bool (val=" << (val ? "true, " : "false, ");

		return ValExpr::prettyPrint(s, buf);
	}

	Byte::Byte(std::string num, int base, Location loc) : Literal{ loc }, val{ std::stoul(num, nullptr, base) } {}
	void Byte::accept(Visitor& v) {
		v.visitByte(*this);
	}
	DEF_PRINTER(Byte) {
		s << std::string(buf, ' ') << context << "ast.Byte (val=" << val << ", ";

		return ValExpr::prettyPrint(s, buf);
	}

	Float::Float(std::string num, Location loc) : Literal{ loc }, val{ std::stof(num) } {}
	void Float::accept(Visitor& v) {
		v.visitFloat(*this);
	}
	DEF_PRINTER(Float) {
		s << std::string(buf, ' ') << context << "ast.Float (val=" << val << ", ";

		return ValExpr::prettyPrint(s, buf);
	}

	Int::Int(std::string num, Location loc) : Literal{ loc }, val{ std::stol(num) } {}
	void Int::accept(Visitor& v) {
		v.visitInt(*this);
	}
	DEF_PRINTER(Int) {
		s << std::string(buf, ' ') << context << "ast.Int (val=" << val << ", ";

		return ValExpr::prettyPrint(s, buf);
	}

	Char::Char(char c, Location loc) : Literal{ loc }, val{ c } {}
	void Char::accept(Visitor& v) {
		v.visitChar(*this);
	}
	DEF_PRINTER(Char) {
		s << std::string(buf, ' ') << context << "ast.Char (val='" << val << "', ";

		return ValExpr::prettyPrint(s, buf);
	}

	String::String(std::string v, Location loc) : ValExpr{ loc }, val{ v } {}
	void String::accept(Visitor& v) {
		v.visitString(*this);
	}
	DEF_PRINTER(String) {
		s << std::string(buf, ' ') << context << "ast.String (val=\"" << val << "\", ";

		return ValExpr::prettyPrint(s, buf);
	}

	Future::Future(bool f, Location loc) : ValExpr{ loc }, generated{ f } {}
	void Future::accept(Visitor& v) {
		v.visitFuture(*this);
	}
	DEF_PRINTER(Future) {
		return s << std::string(buf, ' ') << context << "ast.FutureValue (gen=" << generated << ')';
	}

	Tuple::Tuple(std::deque<ptr<ValExpr>> vals, Location loc) : Sequence{ std::move(vals), loc } {}
	void Tuple::accept(Visitor& v) {
		v.visitTuple(*this);
	}
	DEF_PRINTER(Tuple) {
		s << std::string(buf, ' ') << context << "ast.Tuple" << elems.size() << " (";
		for (auto&& e : elems) {
			e->prettyPrint(s << '\n', buf + 2) << ',';
		}

		if (elems.size()) {
			s << '\n' << std::string(buf, ' ');
		}
		return s << ')';
	}

	Array::Array(std::deque<ptr<ValExpr>> vals, Location loc) : Sequence{ std::move(vals), loc } {}
	void Array::accept(Visitor& v) {
		v.visitArray(*this);
	}
	DEF_PRINTER(Array) {
		s << std::string(buf, ' ') << context << "ast.Array [len=" << elems.size() << ", type={}] [";
		for (auto&& e : elems) {
			e->prettyPrint(s << '\n', buf + 2) << ',';
		}

		if (elems.size()) {
			s << '\n' << std::string(buf, ' ');
		}
		return s << ']';
	}

	Block::Block(std::deque<ptr<Statement>> vals, Location loc) : Sequence{ std::move(vals), loc } {}
	void Block::accept(Visitor& v) {
		v.visitBlock(*this);
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
	void Function::accept(Visitor& v) {
		v.visitFunction(*this);
	}
	DEF_PRINTER(Function) {
		s << std::string(buf, ' ') << context << "ast.Function (";
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
	void BasicBinding::accept(Visitor& v) {
		v.visitBasicBinding(*this);
	}
	DEF_PRINTER(BasicBinding) {
		return s << std::string(buf, ' ') << context << "ast.BasicBinding (var=" << name << ", type=" << type._to_string() << ")";
	}
	std::string BasicBinding::toString() {
		return name;
	}

	QualifiedBinding::QualifiedBinding(ptr<BasicBinding> b, Location loc) : Sequence{ MK_DEQUE(std::move(b)), loc } {}
	QualifiedBinding::QualifiedBinding(std::deque<ptr<BasicBinding>> ps, Location loc) : Sequence{ std::move(ps), loc } {}
	void QualifiedBinding::accept(Visitor& v) {
		v.visitQualifiedBinding(*this);
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


	PathPart::PathPart(std::string n, BindingType t, Location loc)
		: Ast{ loc }, name{ n }, type{ t } {}
	void PathPart::accept(Visitor& v) {
		//v.visitBasicBinding(*this);
	}
	DEF_PRINTER(PathPart) {
		s << name;
		if (gens) s << "[_]";
		return s;
		//return s << std::string(buf, ' ') << context << "ast.PathPart (var=" << name << ", type=" << type._to_string() << ")";
	}

	Path::Path(ptr<PathPart> b, Location loc) : Sequence{ MK_DEQUE(std::move(b)), loc } {}
	void Path::accept(Visitor& v) {
		//v.visitQualifiedBinding(*this);
	}
	DEF_PRINTER(Path) {
		s << std::string(buf, ' ') << context << "ast.Path ";

		elems.front()->prettyPrint(s, 0);
		for (auto p = std::begin(elems) + 1; p != std::end(elems); ++p) {
			p->get()->prettyPrint(s << ':', 0);
		}
		return s;
	}

	Pattern::Pattern(Location loc) : Ast{ loc } {}
	void Pattern::accept(Visitor& v) {
		v.visitPattern(*this);
	}
	DEF_PRINTER(Pattern) {
		return s << std::string(buf, ' ') << context << "ast.AnyPattern (_)";
	}

	TuplePattern::TuplePattern(std::deque<ptr<Pattern>> ps, Location loc)
		: Sequence{ std::move(ps), loc } {}
	void TuplePattern::accept(Visitor& v) {
		v.visitTuplePattern(*this);
	}
	DEF_PRINTER(TuplePattern) {
		s << std::string(buf, ' ') << context << "ast.TuplePattern" << elems.size() << " (capture=" << cap._to_string() << ") (\n";
		for (auto&& p : elems) {
			p->prettyPrint(s, buf + 2) << '\n';
		}
		return s << std::string(buf, ' ') << ')';
	}

	VarPattern::VarPattern(ptr<QualifiedBinding> n, Location loc) : Pattern{ loc }, name{ std::move(n) } {}
	void VarPattern::accept(Visitor& v) {
		v.visitVarPattern(*this);
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
	void AdtPattern::accept(Visitor& v) {
		v.visitAdtPattern(*this);
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
	void ValPattern::accept(Visitor& v) {
		v.visitValPattern(*this);
	}
	DEF_PRINTER(ValPattern) {
		s << std::string(buf, ' ') << context << "ast.PVal";
		return val->prettyPrint(s, 1, "(val=") << ')';
	}

	AssignPattern::AssignPattern(Location loc) : Ast{ loc } {}
	void AssignPattern::accept(Visitor& v) {
		v.visitAssignPattern(*this);
	}
	DEF_PRINTER(AssignPattern) {
		return s << std::string(buf, ' ') << context << "ast.AnyPattern (_)";
	}

	AssignName::AssignName(ptr<BasicBinding> n, Location loc) : AssignPattern{ loc }, var{ std::move(n) } {}
	void AssignName::accept(Visitor& v) {
		v.visitAssignName(*this);
	}
	DEF_PRINTER(AssignName) {
		return var->prettyPrint(s, buf, context);
	}

	AssignTuple::AssignTuple(std::deque<ptr<AssignPattern>> vs, Location loc) : Sequence{ std::move(vs), loc } {}
	void AssignTuple::accept(Visitor& v) {
		v.visitAssignTuple(*this);
	}
	DEF_PRINTER(AssignTuple) {
		s << std::string(buf, ' ') << context << "ast.AssignTuple" << elems.size() << "(\n";
		for (auto&& var : elems) {
			var->prettyPrint(s, buf + 2) << '\n';
		}
		return s << std::string(buf, ' ') << ')';
	}


	//
	// TYPES
	//
	SourceType::SourceType(ptr<QualifiedBinding> b, Location loc)
		: Type{ loc }, name{ std::move(b) }, _ptr{ PtrStyling::NA } {}
	void SourceType::accept(Visitor& v) {
		v.visitSourceType(*this);
	}
	DEF_PRINTER(SourceType) {
		s << std::string(buf, ' ') << context << "ast.SourceType \"";
		return name->prettyPrint(s, 0) << "\" (ptr=" << _ptr._to_string() << ")";
	}

	GenericType::GenericType(ptr<QualifiedBinding> b, ptr<Array> a, Location loc)
		: SourceType{ std::move(b), loc }, inst{ std::move(a) } {}
	void GenericType::accept(Visitor& v) {
		v.visitGenericType(*this);
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
	void TupleType::accept(Visitor& v) {
		v.visitTupleType(*this);
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
	void FunctionType::accept(Visitor& v) {
		v.visitFunctionType(*this);
	}
	DEF_PRINTER(FunctionType) {
		s << std::string(buf, ' ') << context << "ast.FunctionType";

		args->prettyPrint(s << '\n', buf + 2, "args=");
		return ret->prettyPrint(s << '\n', buf + 2, "ret=");
	}

	AndType::AndType(ptr<Type> typs, Location loc)
		: Sequence{ MK_DEQUE<Type>(std::move(typs)), loc } {}
	void AndType::accept(Visitor& v) {
		v.visitAndType(*this);
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
	void OrType::accept(Visitor& v) {
		v.visitOrType(*this);
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
	void Annotation::accept(Visitor& v) {
		v.visitAnnotation(*this);
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
	void LocalAnnotation::accept(Visitor& v) {
		v.visitLocalAnnotation(*this);
	}
	DEF_PRINTER(LocalAnnotation) {
		s << std::string(buf, ' ') << context << "ast.LocalAnnotation name=";
		name->prettyPrint(s, 0);

		if (args) {
			args->prettyPrint(s << '\n', buf + 2, "args=");
		}
		return s;
	}

	GenericPart::GenericPart(ptr<BasicBinding> n, ptr<Type> typ, RelationType rel, Location loc)
		: Ast{ loc }, type{ std::move(typ) }, name{ std::move(n) }, rel{ rel } {}
	void GenericPart::accept(Visitor& v) {
		v.visitGenericPart(*this);
	}

	TypeGeneric::TypeGeneric(ptr<BasicBinding> b, ptr<Type> t, RelationType rel, VarianceType var1, bool var2, Location loc)
		: GenericPart{ std::move(b), std::move(t), rel, loc }, variadic{ var2 }, variance{ var1 } {}
	void TypeGeneric::accept(Visitor& v) {
		v.visitTypeGeneric(*this);
	}
	DEF_PRINTER(TypeGeneric) {
		s << std::string(buf, ' ') << context << "ast.TypeGeneric (rel=" << rel._to_string()
			<< ", variance=" << variance._to_string() << ", variadic=" << variadic << ')';
		name->prettyPrint(s << '\n', buf + 2, "binding=");
		if (type) {
			type->prettyPrint(s << '\n', buf + 2, "type=");
		}
		return s;
	}

	ValueGeneric::ValueGeneric(ptr<BasicBinding> b, ptr<Type> t, RelationType rel, Location loc)
		: GenericPart{ std::move(b), std::move(t), rel, loc } {}
	void ValueGeneric::accept(Visitor& v) {
		v.visitValueGeneric(*this);
	}
	DEF_PRINTER(ValueGeneric) {
		s << std::string(buf, ' ') << context << "ast.ValueGeneric (rel=" << rel._to_string() << ')';
		name->prettyPrint(s << '\n', buf + 2, "binding=");
		if (type) {
			type->prettyPrint(s << '\n', buf + 2, "type=");
		}
		return s;
	}

	GenericArray::GenericArray(std::deque<ptr<GenericPart>> elems, Location loc)
		: Sequence{ std::move(elems), loc } {}
	void GenericArray::accept(Visitor& v) {
		v.visitGenericArray(*this);
	}
	DEF_PRINTER(GenericArray) {
		s << std::string(buf, ' ') << "ast.Generic (size=" << elems.size() << ") [";

		for (auto&& part : elems) {
			part->prettyPrint(s << '\n', buf + 2);
		}

		return s << '\n' << std::string(buf, ' ') << ']';
	}

	Constructor::Constructor(Location loc) : Ast{ loc } {}

	Adt::Adt(ptr<BasicBinding> n, ptr<TupleType> t, Location loc)
		: Constructor{ loc }, name{ std::move(n) }, args{ std::move(t) } {}
	void Adt::accept(Visitor& v) {
		v.visitAdt(*this);
	}
	DEF_PRINTER(Adt) {
		s << std::string(buf, ' ') << context << "ast.Adt (name=";
		name->prettyPrint(s, 0) << ")";

		if (args) {
			args->prettyPrint(s << '\n', buf + 2, "args=");
		}
		return s;
	}

	Argument::Argument(ptr<BasicBinding> b, ptr<Type> t, Location loc)
		: Ast{ loc }, name{ std::move(b) }, typ{ std::move(t) } {}
	void Argument::accept(Visitor& v) {
		v.visitArgument(*this);
	}
	DEF_PRINTER(Argument) {
		s << std::string(buf, ' ') << context << "ast.Arg (name=";
		name->prettyPrint(s, 0) << ")";

		if (typ) {
			typ->prettyPrint(s << '\n', buf + 2, "type=");
		}
		return s;
	}

	ArgTuple::ArgTuple(std::deque<ptr<Argument>> args, Location loc) : Sequence{ std::move(args), loc } {}
	void ArgTuple::accept(Visitor& v) {
		v.visitArgTuple(*this);
	}
	DEF_PRINTER(ArgTuple) {
		s << std::string(buf, ' ') << context << "ast.ArgumentList (size=" << elems.size() << ')';
		for (auto&& arg : elems) {
			arg->prettyPrint(s << '\n', buf + 2);
		}
		return s;
	}


	//
	// CONTROL
	//

	Branch::Branch(Location loc) : ValExpr{ loc } {}
	void Branch::accept(Visitor& v) {
		v.visitBranch(*this);
	}

	Loop::Loop(ptr<ValExpr> b, Location loc) : Branch{ loc }, body{ std::move(b) } {}
	void Loop::accept(Visitor& v) {
		v.visitLoop(*this);
	}
	DEF_PRINTER(Loop) {
		s << std::string(buf, ' ') << context << "ast.Loop (";
		ValExpr::prettyPrint(s, buf);
		return body->prettyPrint(s << '\n', buf + 2, "body=");
	}

	While::While(ptr<ValExpr> test, ptr<ValExpr> body, Location loc)
		: Loop{ std::move(body), loc }, test{ std::move(test) } {}
	void While::accept(Visitor& v) {
		v.visitWhile(*this);
	}
	DEF_PRINTER(While) {
		s << std::string(buf, ' ') << context << "ast.While (";
		ValExpr::prettyPrint(s, buf);
		test->prettyPrint(s << '\n', buf + 2, "cond=");
		return body->prettyPrint(s << '\n', buf + 2, "body=");
	}

	For::For(ptr<Pattern> p, ptr<ValExpr> g, ptr<ValExpr> b, Location loc)
		: Loop{ std::move(b), loc }, pattern{ std::move(p) }, generator{ std::move(g) } {}
	void For::accept(Visitor& v) {
		v.visitFor(*this);
	}
	DEF_PRINTER(For) {
		s << std::string(buf, ' ') << context << "ast.For (";
		ValExpr::prettyPrint(s, buf);

		pattern->prettyPrint(s << '\n', buf + 2, "vars=");
		generator->prettyPrint(s << '\n', buf + 2, "in=");
		return body->prettyPrint(s << '\n', buf + 2, "body=");
	}

	IfBranch::IfBranch(ptr<ValExpr> test, ptr<ValExpr> body, bool eif, Location loc)
		: Branch{ loc }, test{ std::move(test) }, body{ std::move(body) }, elsif{ eif } {}
	void IfBranch::accept(Visitor& v) {
		v.visitIfBranch(*this);
	}
	DEF_PRINTER(IfBranch) {
		s << std::string(buf, ' ') << context << (elsif ? "ast.Elsif (" : "ast.If (");
		ValExpr::prettyPrint(s, buf);

		test->prettyPrint(s << '\n', buf + 2, "test=");
		return body->prettyPrint(s << '\n', buf + 2, "body=");
	}

	IfElse::IfElse(std::deque<ptr<IfBranch>> ifs, ptr<ValExpr> _else, Location loc)
		: Sequence{ std::move(ifs), loc }, _else_{ std::move(_else) } {}
	void IfElse::accept(Visitor& v) {
		v.visitIfElse(*this);
	}
	DEF_PRINTER(IfElse) {
		s << std::string(buf, ' ') << context << "ast.Branch (";
		ValExpr::prettyPrint(s, buf);

		for (auto&& _if_ : elems) {
			_if_->prettyPrint(s << '\n', buf + 2);
		}

		if (_else_) {
			_else_->prettyPrint(s << '\n', buf + 2, "else=");
		}

		return s;
	}

	Case::Case(ptr<TuplePattern> vs, ptr<ValExpr> if_g, ptr<ValExpr> e, Location loc)
		: ValExpr{ loc }, vars{ std::move(vs) }, expr{ std::move(e) }, if_guard{ std::move(if_g) } {}
	void Case::accept(Visitor& v) {
		v.visitCase(*this);
	}
	DEF_PRINTER(Case) {
		s << std::string(buf, ' ') << context << "ast.Case";
		vars->prettyPrint(s << '\n', buf + 2, "pattern=");
		if (if_guard) {
			if_guard->prettyPrint(s << '\n', buf + 2, "if=");
		}
		return expr->prettyPrint(s << '\n', buf + 2, "expr=");
	}

	Match::Match(ptr<ValExpr> s, std::deque<ptr<Case>> cs, Location loc)
		: Branch{ loc }, switch_expr{ std::move(s) }, cases{ std::move(cs) } {}
	void Match::accept(Visitor& v) {
		v.visitMatch(*this);
	}
	DEF_PRINTER(Match) {
		s << std::string(buf, ' ') << context << "ast.Match (";
		ValExpr::prettyPrint(s, buf);

		switch_expr->prettyPrint(s << '\n', buf + 2, "switch=");

		for (auto&& c : cases) {
			c->prettyPrint(s << '\n', buf + 2, "case=");
		}

		return s;
	}

	Jump::Jump(KeywordType t, ptr<ValExpr> e, Location loc)
		: Branch{ loc }, type{ t }, expr{ std::move(e) } {}
	void Jump::accept(Visitor& v) {
		v.visitJump(*this);
	}
	DEF_PRINTER(Jump) {
		s << std::string(buf, ' ') << context << "ast.Jump (type=" << type._to_string();
		ValExpr::prettyPrint(s << ", ", buf);

		if (expr) {
			expr->prettyPrint(s << '\n', buf + 2, "expr=");
		}
		return s;
	}


	//
	// STMTS
	//

	ModDec::ModDec(ptr<QualifiedBinding> v, Location loc) : Statement{ loc }, module{ std::move(v) } {}
	void ModDec::accept(Visitor& v) {
		v.visitModDec(*this);
	}
	DEF_PRINTER(ModDec) {
		s << std::string(buf, ' ') << context << "ast.ModuleDec (module=";
		module->prettyPrint(s, 0) << ')';
		return Statement::prettyPrint(s, buf + 2);
	}

	ImplExpr::ImplExpr(ptr<SourceType> t, ptr<SourceType> inter, ptr<Block> b, Location loc)
		: Statement{ loc }, type{ std::move(t) }, _interface{ std::move(inter) }, impls{ std::move(b) } {}
	void ImplExpr::accept(Visitor& v) {
		v.visitImplExpr(*this);
	}
	DEF_PRINTER(ImplExpr) {
		s << std::string(buf, ' ') << context << "ast.ImplExpr\n";
		_interface->prettyPrint(s, buf + 2, "type=");
		if (type) {
			type->prettyPrint(s << '\n', buf + 2, "for=");
		}
		if (impls) {
			impls->prettyPrint(s << '\n', buf + 2, "impl=");
		}
		return Statement::prettyPrint(s, buf + 2);
	}

	ModRebindImport::ModRebindImport(Location loc) : Statement{ loc }, _module{ nullptr } {}
	ModRebindImport::ModRebindImport(ptr<QualifiedBinding> mod, Location loc)
		: Statement{ loc }, _module{ std::move(mod) } {}
	DEF_PRINTER(ModRebindImport) {
		if (_module) {
			_module->prettyPrint(s, buf, context);
		} else {
			s << std::string(buf, ' ') << context << "{scope}";
		}

		return s;
	}

	SingleImport::SingleImport(ptr<QualifiedBinding> mod, ptr<BasicBinding> name, Location loc)
		: ModRebindImport{ std::move(mod), loc }, binding{ std::move(name) } {}
	void SingleImport::accept(Visitor& v) {
		v.visitSingleImport(*this);
	}
	DEF_PRINTER(SingleImport) {
		s << std::string(buf, ' ') << context << "ast.SingleImport";
		Statement::prettyPrint(s, buf + 2);
		ModRebindImport::prettyPrint(s << '\n', buf + 2, "from=");

		return binding->prettyPrint(s << '\n', buf + 2, "import=");
	}

	MultipleImport::MultipleImport(ptr<QualifiedBinding> mod, std::deque<ptr<BasicBinding>> names, Location loc)
		: Sequence{ std::move(names), loc } {
		_module = std::move(mod);
	}
	void MultipleImport::accept(Visitor& v) {
		v.visitMultipleImport(*this);
	}
	DEF_PRINTER(MultipleImport) {
		s << std::string(buf, ' ') << context << "ast.MultipleImport";
		Statement::prettyPrint(s, buf + 2);
		ModRebindImport::prettyPrint(s << '\n', buf + 2, "from=");

		s << '\n' << std::string(buf + 2, ' ') << "import= {";

		for (auto&& name : elems) {
			name->prettyPrint(s << '\n', buf + 4);
		}

		return s << '\n' << std::string(buf + 2, ' ') << '}';
	}

	PathMultipleImport::PathMultipleImport(ptr<Path> mod, std::deque<ptr<PathPart>> names, Location loc)
		: Sequence{ std::move(names), loc }, _tmp_module{ std::move(mod) } {}
	void PathMultipleImport::accept(Visitor& v) {
		//v.visitMultipleImport(*this);
	}
	DEF_PRINTER(PathMultipleImport) {
		s << std::string(buf, ' ') << context << "ast.MultipleImport";
		Statement::prettyPrint(s, buf + 2);
		if (_tmp_module) {
			_tmp_module->prettyPrint(s, buf+2, "from=");
		} else {
			s << std::string(buf+2, ' ') << "from={scope}";
		}
		//ModRebindImport::prettyPrint(s << '\n', buf + 2, "from=");

		s << '\n' << std::string(buf + 2, ' ') << "import= {";

		for (auto&& name : elems) {
			name->prettyPrint(s, 0) << ", ";
		}

		return s << '}';
	}

	Rebind::Rebind(ptr<QualifiedBinding> mod, ptr<BasicBinding> bind, ptr<Array> arr, ptr<BasicBinding> nbind, ptr<Array> narr, Location loc)
		: ModRebindImport{ std::move(mod), loc }, old_name{ std::move(bind) }, old_gen{ std::move(arr) }, new_name{ std::move(nbind) }, new_gen{ std::move(narr) } {}
	void Rebind::accept(Visitor& v) {
		v.visitRebind(*this);
	}
	DEF_PRINTER(Rebind) {
		s << std::string(buf, ' ') << context << "ast.Rebind";
		Statement::prettyPrint(s, buf + 2);
		ModRebindImport::prettyPrint(s << '\n', buf + 2, "from=");

		old_name->prettyPrint(s << '\n', buf + 2, "rebind=");
		if (old_gen) {
			old_gen->prettyPrint(s << '\n', buf + 4, "inst=");
		}

		new_name->prettyPrint(s << '\n', buf + 2, "as=");
		if (new_gen) {
			new_gen->prettyPrint(s << '\n', buf + 4, "inst=");
		}

		return s;
	}

	Interface::Interface(VisibilityType vis, ptr<AssignPattern> n, ptr<GenericArray> gs, ptr<Type> t, Location loc)
		: Statement{ loc }, vis{ vis }, name{ std::move(n) }, gen{ std::move(gs) }, type{ std::move(t) } {}
	void Interface::accept(Visitor& v) {
		v.visitInterface(*this);
	}
	DEF_PRINTER(Interface) {
		s << std::string(buf, ' ') << context << "ast.Interface (vis=" << vis._to_string() << ')';
		name->prettyPrint(s << '\n', buf + 2, "var=");

		if (type) {
			type->prettyPrint(s << '\n', buf + 2, "type=");
		}
		if (gen) {
			gen->prettyPrint(s << '\n', buf + 2);
		}

		return s;
	}

	TypeAssign::TypeAssign(VisibilityType vis, ptr<AssignPattern> n, ptr<GenericArray> gs, bool m, ConsList cs, ptr<Block> b, Location loc)
		: Interface{ vis, std::move(n), std::move(gs), nullptr, loc }, cons{ std::move(cs) }, body{ std::move(b) }, mutable_only{ m } {}
	void TypeAssign::accept(Visitor& v) {
		v.visitTypeAssign(*this);
	}
	DEF_PRINTER(TypeAssign) {
		s << std::string(buf, ' ') << context << "ast.TypeAssign (vis=" << vis._to_string() << ", mut=" << mutable_only << ')';
		name->prettyPrint(s << '\n', buf + 2, "var=");

		if (type) {
			type->prettyPrint(s << '\n', buf + 2, "type=");
		}
		if (gen) {
			gen->prettyPrint(s << '\n', buf + 2);
		}

		if (cons.size()) {
			s << '\n' << std::string(buf + 2, ' ') << "ast.Constructors [";

			for (auto&& con : cons) {
				con->prettyPrint(s << '\n', buf + 4);
			}

			s << '\n' << std::string(buf + 2, ' ') << ']';
		}

		return body->prettyPrint(s << '\n', buf + 2, "body=");
	}

	VarAssign::VarAssign(VisibilityType vis, ptr<AssignPattern> n, ptr<GenericArray> gs, ptr<Type> t, ptr<ValExpr> v, Location loc)
		: Interface{ vis, std::move(n), std::move(gs), std::move(t), loc }, expr{ std::move(v) } {}
	void VarAssign::accept(Visitor& v) {
		v.visitVarAssign(*this);
	}
	DEF_PRINTER(VarAssign) {
		s << std::string(buf, ' ') << context << "ast.VarAssign (vis=" << vis._to_string() << ')';
		name->prettyPrint(s << '\n', buf + 2, "var=");

		if (type) {
			type->prettyPrint(s << '\n', buf + 2, "type=");
		}
		if (gen) {
			gen->prettyPrint(s << '\n', buf + 2);
		}

		return expr->prettyPrint(s << '\n', buf + 2, "value=");
	}

	TypeExtension::TypeExtension(ptr<QualifiedBinding> t, ptr<Array> g, ptr<Tuple> a, ptr<Block> e, Location loc)
		: ValExpr{ loc }, typ_name{ std::move(t) }, gen{ std::move(g) }, args{ std::move(a) }, ext{ std::move(e) } {}
	void TypeExtension::accept(Visitor& v) {
		v.visitTypeExtension(*this);
	}
	DEF_PRINTER(TypeExtension) {
		s << std::string(buf, ' ') << context << "ast.AnonymousType";
		typ_name->prettyPrint(s << '\n', buf + 2, "extend=");

		if (gen) {
			gen->prettyPrint(s << '\n', buf + 4);
		}
		if (args) {
			args->prettyPrint(s << '\n', buf + 4);
		}

		return ext->prettyPrint(s << '\n', buf + 2, "with=");
	}


	//
	// EXPRESSIONS
	//
	Variable::Variable(ptr<QualifiedBinding> n, ptr<Array> i, Location loc)
		: ValExpr{ loc }, name{ std::move(n) }, inst_args{ std::move(i) } {}
	void Variable::accept(Visitor& v) {
		v.visitVariable(*this);
	}
	DEF_PRINTER(Variable) {
		s << std::string(buf, ' ') << context << "ast.Variable (";
		ValExpr::prettyPrint(s, buf);
		name->prettyPrint(s << "\n", buf + 2, "name=");

		if (inst_args) {
			inst_args->prettyPrint(s << '\n', buf + 2, "inst=");
		}

		return s;
	}

	/*UnOpCall::UnOpCall(ptr<ValExpr> e, std::string op, Location loc)
		: ValExpr{ loc }, expr{ std::move(e) }, rhs{ std::move(rhs) }, op{ op } {}*/
	UnOpCall::UnOpCall(ptr<ValExpr> e, ptr<BasicBinding> op, Location loc)
		: ValExpr{ loc }, expr{ std::move(e) }, op{ std::move(op) } {}
	void UnOpCall::accept(Visitor& v) {
		v.visitUnOpCall(*this);
	}
	DEF_PRINTER(UnOpCall) {
		//s << std::string(buf, ' ') << context << "ast.UnOpCall (op=" << op;
		s << std::string(buf, ' ') << context << "ast.UnOpCall (op=";
		op->prettyPrint(s, 0);
		ValExpr::prettyPrint(s << ", ", buf);
		return expr->prettyPrint(s << '\n', buf + 2, "lhs=");
	}

	BinOpCall::BinOpCall(ptr<ValExpr> lhs, ptr<ValExpr> rhs, std::string op, Location loc)
	: ValExpr{ loc }, lhs{ std::move(lhs) }, rhs{ std::move(rhs) }, op{ op } {}
	/*BinOpCall::BinOpCall(ptr<ValExpr> lhs, ptr<ValExpr> rhs, ptr<BasicBinding> op, Location loc)
		: ValExpr{ loc }, lhs{ std::move(lhs) }, rhs{ std::move(rhs) }, op{ std::move(op) } {}*/
	void BinOpCall::accept(Visitor& v) {
		v.visitBinOpCall(*this);
	}
	DEF_PRINTER(BinOpCall) {
		s << std::string(buf, ' ') << context << "ast.BinOpCall (op=" << op;
		//op->prettyPrint(s << std::string(buf, ' ') << context << "ast.BinOpCall (op=", 0);
		ValExpr::prettyPrint(s << ", ", buf);
		lhs->prettyPrint(s << '\n', buf + 2, "lhs=");
		return rhs->prettyPrint(s << '\n', buf + 2, "rhs=");
	}

	InAssign::InAssign(ptr<VarAssign> v, ptr<ValExpr> e, Location loc)
		: ValExpr{ loc }, bind{ std::move(v) }, expr{ std::move(e) } {}
	void InAssign::accept(Visitor& v) {
		v.visitInAssign(*this);
	}
	DEF_PRINTER(InAssign) {
		s << std::string(buf, ' ') << context << "ast.ScopedBinding\n";
		bind->prettyPrint(s, buf + 2, "bind=") << '\n';
		return expr->prettyPrint(s, buf + 2, "in=");
	}

	Index::Index(std::deque<ptr<ValExpr>> els, Location loc)
		: Sequence{ std::move(els), loc } {}
	void Index::accept(Visitor& v) {
		v.visitIndex(*this);
	}
	DEF_PRINTER(Index) {
		s << std::string(buf, ' ') << context << "ast.IndexSequence (";
		ValExpr::prettyPrint(s, buf);

		for (auto&& e : elems) {
			e->prettyPrint(s << '\n', buf + 2, "idx=");
		}

		return s;
	}

	FnCall::FnCall(ptr<ValExpr> c, ptr<Tuple> a, Location loc)
		: ValExpr{ loc }, callee{ std::move(c) }, arguments{ std::move(a) } {}
	void FnCall::accept(Visitor& v) {
		v.visitFnCall(*this);
	}
	DEF_PRINTER(FnCall) {
		s << std::string(buf, ' ') << context << "ast.FnCall (args="
			<< (bool)arguments;
		ValExpr::prettyPrint(s, buf) << '\n';
		callee->prettyPrint(s, buf + 2, "caller=");

		if (arguments) {
			arguments->prettyPrint(s << '\n', buf + 2, "args=");
		}
		return s;
	}


	//
	// ERRORS
	//

	Symbol::Symbol(char c, Location loc) : Ast{ loc }, ch{ c } {}

	Error::Error(Location loc) : Ast{ loc } {}
	DEF_PRINTER(Error) {
		return s << std::string(buf, ' ') << context << "ast.Error";
	}

	CloseSymbolError::CloseSymbolError(Location loc) : Error{ loc } {}

	ValError::ValError(Location loc) : ValExpr{ loc } {}
	DEF_PRINTER(ValError) {
		return s << std::string(buf, ' ') << context << "ast.Error";
	}

	ImportError::ImportError(Location loc) : ModRebindImport{ loc } {}
	void ImportError::accept(Visitor&) {}
}