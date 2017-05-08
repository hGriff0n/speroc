#include "parser/visitor.h"

namespace spero::compiler::ast {
	// Only defined as `std::deque` has no `initializer_list` constructor
	template<class T>
	std::deque<ptr<T>> MK_DEQUE(ptr<T> e) { std::deque<ptr<T>> ret; ret.emplace_back(std::move(e)); return std::move(ret); }
	template<class T>
	std::deque<ptr<T>> MK_DEQUE(ptr<T> e1, ptr<T> e2) { std::deque<ptr<T>> ret; ret.emplace_back(std::move(e1)); ret.emplace_back(std::move(e2)); return std::move(ret); }

	// Helper define to reduce typing
#define DEF_PRINTER(typ) std::ostream& typ::prettyPrint(std::ostream& s, size_t buf, std::string context)


/*
 * AST class implementations
 */
	Ast::Ast(Ast::Location loc) : loc{ loc } {}
	Visitor& Ast::visit(Visitor& v) { v.accept(*this); return v; }
	DEF_PRINTER(Ast) {
		return s << std::string(buf, ' ') << context << "ast.Ast";
	}

	Visitor& Token::visit(Visitor& v) {
		v.acceptToken(*this);
		return v;
	}
	DEF_PRINTER(Token) {
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

	Type::Type(Ast::Location loc) : Ast{ loc } {}
	Visitor& Type::visit(Visitor& v) {
		v.acceptType(*this);
		return v;
	}

	Stmt::Stmt(Ast::Location loc) : Ast{ loc } {}
	Visitor& Stmt::visit(Visitor& v) {
		v.acceptStmt(*this);
		return v;
	}
	DEF_PRINTER(Stmt) {
		for (auto&& a : annots)
			a->prettyPrint(s << '\n', buf + 2);

		return s;
	}

	ValExpr::ValExpr(Ast::Location loc) : Stmt{ loc } {}
	Visitor& ValExpr::visit(Visitor& v) {
		v.acceptValExpr(*this);
		return v;
	}
	DEF_PRINTER(ValExpr) {
		s << "mut=" << is_mut << ')';

		if (type)
			type->prettyPrint(s << '\n', buf + 2, "type=");

		return Stmt::prettyPrint(s, buf);
	}

	Bool::Bool(bool b, Ast::Location loc) : ValExpr{ loc }, val{ b } {}
	Visitor& Bool::visit(Visitor& v) {
		v.acceptBool(*this);
		return v;
	}
	DEF_PRINTER(Bool) {
		s << std::string(buf, ' ') << context << "ast.Bool (val=" << (val ? "true, " : "false, ");

		return ValExpr::prettyPrint(s, buf);
	}

	Byte::Byte(std::string num, int base, Ast::Location loc) : ValExpr{ loc }, val{ std::stoul(num, nullptr, base) } {}
	Visitor& Byte::visit(Visitor& v) {
		v.acceptByte(*this);
		return v;
	}
	DEF_PRINTER(Byte) {
		s << std::string(buf, ' ') << context << "ast.Byte (val=" << val << ", ";

		return ValExpr::prettyPrint(s, buf);
	}

	Float::Float(std::string num, Ast::Location loc) : ValExpr{ loc }, val{ std::stof(num) } {}
	Visitor& Float::visit(Visitor& v) {
		v.acceptFloat(*this);
		return v;
	}
	DEF_PRINTER(Float) {
		s << std::string(buf, ' ') << context << "ast.Float (val=" << val << ", ";

		return ValExpr::prettyPrint(s, buf);
	}

	Int::Int(std::string num, Ast::Location loc) : ValExpr{ loc }, val{ std::stoi(num) } {}
	Visitor& Int::visit(Visitor& v) {
		v.acceptInt(*this);
		return v;
	}
	DEF_PRINTER(Int) {
		s << std::string(buf, ' ') << context << "ast.Int (val=" << val << ", ";

		return ValExpr::prettyPrint(s, buf);
	}

	Char::Char(char c, Ast::Location loc) : ValExpr{ loc }, val{ c } {}
	Visitor& Char::visit(Visitor& v) {
		v.acceptChar(*this);
		return v;
	}
	DEF_PRINTER(Char) {
		s << std::string(buf, ' ') << context << "ast.Char (val='" << val << "', ";

		return ValExpr::prettyPrint(s, buf);
	}

	String::String(std::string v, Ast::Location loc) : ValExpr{ loc }, val{ v } {}
	Visitor& String::visit(Visitor& v) {
		v.acceptString(*this);
		return v;
	}
	DEF_PRINTER(String) {
		s << std::string(buf, ' ') << context << "ast.String (val=\"" << val << "\", ";

		return ValExpr::prettyPrint(s, buf);
	}

	Tuple::Tuple(std::deque<ptr<ValExpr>> vals, Ast::Location loc) : Sequence{ std::move(vals), loc } {}
	Visitor& Tuple::visit(Visitor& v) {
		v.acceptTuple(*this);
		return v;
	}
	DEF_PRINTER(Tuple) {
		s << std::string(buf, ' ') << context << "ast.Tuple" << elems.size() << " (";
		for (auto&& e : elems)
			e->prettyPrint(s << '\n', buf + 2) << ",";

		if (elems.size()) s << '\n' << std::string(buf, ' ');
		return s << ')';
	}

	Array::Array(std::deque<ptr<ValExpr>> vals, Ast::Location loc) : Sequence{ std::move(vals), loc } {}
	Visitor& Array::visit(Visitor& v) {
		v.acceptArray(*this);
		return v;
	}
	DEF_PRINTER(Array) {
		s << std::string(buf, ' ') << context << "ast.Array" << elems.size() << " [type={}] [";
		for (auto&& e : elems)
			e->prettyPrint(s << '\n', buf + 2) << ",";

		if (elems.size()) s << '\n' << std::string(buf, ' ');
		return s << ']';
	}

	Block::Block(std::deque<ptr<Stmt>> vals, Ast::Location loc) : Sequence{ std::move(vals), loc } {}
	Visitor& Block::visit(Visitor& v) {
		v.acceptBlock(*this);
		return v;
	}
	DEF_PRINTER(Block) {
		s << std::string(buf, ' ') << context << "ast.Block (size=" << elems.size() << ") {";
		for (auto&& e : elems)
			e->prettyPrint(s << '\n', buf + 2);

		if (elems.size()) s << '\n' << std::string(buf, ' ');
		return s << '}';
	}

	BasicBinding::BasicBinding(std::string n, BindingType t, Ast::Location loc)
		: Ast{ loc }, name{ n }, type{ t } {}
	Visitor& BasicBinding::visit(Visitor& v) {
		v.acceptBasicBinding(*this);
		return v;
	}
	DEF_PRINTER(BasicBinding) {
		return s << std::string(buf, ' ') << context << "ast.BasicBinding (var=" << name << ", type=" << type._to_string() << ")";
	}

	QualifiedBinding::QualifiedBinding(ptr<BasicBinding> b, Ast::Location loc) : Sequence{ MK_DEQUE(std::move(b)), loc } {}
	QualifiedBinding::QualifiedBinding(std::deque<ptr<BasicBinding>> ps, Ast::Location loc) : Sequence{ std::move(ps), loc } {}
	Visitor& QualifiedBinding::visit(Visitor& v) {
		v.acceptQualifiedBinding(*this);
		return v;
	}
	DEF_PRINTER(QualifiedBinding) {
		s << std::string(buf, ' ') << context << "ast.QualifiedBinding " << elems.front()->name;

		for (auto p = std::begin(elems) + 1; p != std::end(elems); ++p)
			s << ":" << p->get()->name;

		return s;
	}

	Variable::Variable(ptr<QualifiedBinding> n, Ast::Location loc) : ValExpr{ loc }, name{ std::move(n) } {}
	Visitor& Variable::visit(Visitor& v) {
		v.acceptVariable(*this);
		return v;
	}
	DEF_PRINTER(Variable) {
		s << std::string(buf, ' ') << context << "ast.Variable (";
		ValExpr::prettyPrint(s, buf);
		return name->prettyPrint(s << "\n", buf + 2, "name=");
	}

	AssignPattern::AssignPattern(Ast::Location loc) : Ast{ loc } {}
	Visitor& AssignPattern::visit(Visitor& v) {
		v.acceptAssignPattern(*this);
		return v;
	}

	AssignName::AssignName(ptr<BasicBinding> n, Ast::Location loc) : AssignPattern{ loc }, var{ std::move(n) } {}
	Visitor& AssignName::visit(Visitor& v) {
		v.acceptAssignName(*this);
		return v;
	}
	DEF_PRINTER(AssignName) {
		return var->prettyPrint(s, buf, context);
	}

	AssignTuple::AssignTuple(std::deque<ptr<AssignPattern>> vs, Ast::Location loc) : Sequence{ std::move(vs), loc } {}
	Visitor& AssignTuple::visit(Visitor& v) {
		v.acceptAssignTuple(*this);
		return v;
	}
	DEF_PRINTER(AssignTuple) {
		s << std::string(buf, ' ') << context << "ast.AssignTuple" << elems.size() << "(\n";
		for (auto&& var : elems)
			var->prettyPrint(s, buf + 2) << '\n';
		return s << std::string(buf, ' ') << ')';
	}

	Pattern::Pattern(Ast::Location loc) : Ast{ loc } {}
	Visitor& Pattern::visit(Visitor& v) {
		v.acceptPattern(*this);
		return v;
	}
	DEF_PRINTER(Pattern) {
		return s << std::string(buf, ' ') << context << "ast.PatternAny (_)";
	}

	PTuple::PTuple(std::deque<ptr<Pattern>> ps, Ast::Location loc) : Sequence{ std::move(ps), loc } {}
	Visitor& PTuple::visit(Visitor& v) {
		v.acceptPTuple(*this);
		return v;
	}
	DEF_PRINTER(PTuple) {
		s << std::string(buf, ' ') << context << "ast.PTuple" << elems.size() << " (capture=" << cap._to_string() << ") (\n";
		for (auto&& p : elems)
			p->prettyPrint(s, buf + 2) << '\n';
		return s << std::string(buf, ' ') << ')';
	}

	PNamed::PNamed(ptr<BasicBinding> n, ptr<Type> typ, Ast::Location loc) : Pattern{ loc }, name{ std::move(n) }, type{ std::move(typ) } {}
	Visitor& PNamed::visit(Visitor& v) {
		v.acceptPNamed(*this);
		return v;
	}
	DEF_PRINTER(PNamed) {
		s << std::string(buf, ' ') << context << "ast.PNamed (capture=" << cap._to_string() << ')';
		name->prettyPrint(s, 1);
		if (type) type->prettyPrint(s << '\n', buf + 2, "type=");

		return s;
	}

	PAdt::PAdt(ptr<BasicBinding> n, ptr<PTuple> as, Ast::Location loc)
		: PNamed{ std::move(n), nullptr, loc }, args{ std::move(as) } {}
	Visitor& PAdt::visit(Visitor& v) {
		v.acceptPAdt(*this);
		return v;
	}
	DEF_PRINTER(PAdt) {
		s << std::string(buf, ' ') << context << "ast.PAdt";
		name->prettyPrint(s, 1, "(type=") << ')';
		if (args) args->prettyPrint(s << '\n', buf + 2);
		return s;
	}

	PVal::PVal(ptr<ValExpr> v, Ast::Location loc) : Pattern{ loc }, val{ std::move(v) } {}
	Visitor& PVal::visit(Visitor& v) {
		v.acceptPVal(*this);
		return v;
	}
	DEF_PRINTER(PVal) {
		s << std::string(buf, ' ') << context << "ast.PVal";
		return val->prettyPrint(s, 1, "(val=") << ')';
	}

	SourceType::SourceType(ptr<BasicBinding> b, PtrStyling p, Ast::Location loc)
		: SourceType{ std::make_unique<QualifiedBinding>(std::move(b), loc), p, loc } {}
	SourceType::SourceType(ptr<QualifiedBinding> b, PtrStyling p, Ast::Location loc)
		: Type{ loc }, name{ std::move(b) }, _ptr{ p } {}
	Visitor& SourceType::visit(Visitor& v) {
		v.acceptSourceType(*this);
		return v;
	}
	DEF_PRINTER(SourceType) {
		s << std::string(buf, ' ') << context << "ast.SourceType \"";
		return name->prettyPrint(s, 0) << "\" (ptr=" << _ptr._to_string() << ")";
	}

	GenericType::GenericType(ptr<QualifiedBinding> b, ptr<Array> a, PtrStyling p, Ast::Location loc)
		: SourceType{ std::move(b), p, loc }, inst{ std::move(a) } {}
	Visitor& GenericType::visit(Visitor& v) {
		v.acceptGenericType(*this);
		return v;
	}
	DEF_PRINTER(GenericType) {
		s << std::string(buf, ' ') << context << "ast.GenericType \"";
		name->prettyPrint(s, 0) << "\" (ptr=" << _ptr._to_string() << ")";

		if (inst && inst->elems.size()) {
			s << " [\n";
			for (auto&& e : inst->elems)
				e->prettyPrint(s, buf + 2) << '\n';
			s << std::string(buf, ' ') << "]";
		}

		return s;
	}

	TupleType::TupleType(std::deque<ptr<Type>> ts, Ast::Location loc) : Sequence{ std::move(ts), loc } {}
	Visitor& TupleType::visit(Visitor& v) {
		v.acceptTupleType(*this);
		return v;
	}
	DEF_PRINTER(TupleType) {
		s << std::string(buf, ' ') << context << "ast.TupleType" << elems.size() << "(";

		for (auto&& t : elems)
			t->prettyPrint(s << '\n', buf + 2);

		return s << '\n' << std::string(buf, ' ') << ')';
	}

	FuncType::FuncType(ptr<TupleType> as, ptr<Type> ret, Ast::Location loc)
		: Type{ loc }, args{ std::move(as) }, ret{ std::move(ret) } {}
	Visitor& FuncType::visit(Visitor& v) {
		v.acceptFuncType(*this);
		return v;
	}
	DEF_PRINTER(FuncType) {
		s << std::string(buf, ' ') << context << "ast.FunctionType";

		args->prettyPrint(s << '\n', buf + 2, "args=");
		return ret->prettyPrint(s << '\n', buf + 2, "ret=");
	}

	AndType::AndType(std::deque<ptr<Type>> typs, Ast::Location loc)
		: Type{ loc }, types{ std::move(typs) } {}
	Visitor& AndType::visit(Visitor& v) {
		v.acceptAndType(*this);
		return v;
	}
	DEF_PRINTER(AndType) {
		s << std::string(buf, ' ') << context << "ast.AndType";

		for (auto&& t : types)
			t->prettyPrint(s << '\n', buf + 2);

		return s;
	}

	OrType::OrType(std::deque<ptr<Type>> typs, Ast::Location loc)
		: Type{ loc }, types{ std::move(typs) } {}
	Visitor& OrType::visit(Visitor& v) {
		v.acceptOrType(*this);
		return v;
	}
	DEF_PRINTER(OrType) {
		s << std::string(buf, ' ') << context << "ast.OrType";

		for (auto&& t : types)
			t->prettyPrint(s << '\n', buf + 2);

		return s;
	}

	Annotation::Annotation(ptr<BasicBinding> n, ptr<Tuple> t, Ast::Location loc)
		: Ast{ loc }, name{ std::move(n) }, args{ std::move(t) } {}
	Visitor& Annotation::visit(Visitor& v) {
		v.acceptAnnotation(*this);
		return v;
	}
	DEF_PRINTER(Annotation) {
		s << std::string(buf, ' ') << context << "ast.Annotation name=";
		name->prettyPrint(s, 0);

		if (args) args->prettyPrint(s << '\n', buf + 2, "args=");
		return s;
	}

	LocalAnnotation::LocalAnnotation(ptr<BasicBinding> n, ptr<Tuple> t, Ast::Location loc)
		: Annotation{ std::move(n), std::move(t), loc } {}
	Visitor& LocalAnnotation::visit(Visitor& v) {
		v.acceptLocalAnnotation(*this);
		return v;
	}
	DEF_PRINTER(LocalAnnotation) {
		s << std::string(buf, ' ') << context << "ast.LocalAnnotation name=";
		name->prettyPrint(s, 0);

		if (args) args->prettyPrint(s << '\n', buf + 2, "args=");
		return s;
	}

	GenericPart::GenericPart(ptr<BasicBinding> n, Ast::Location loc)
		: Ast{ loc }, name{ std::move(n) } {}
	Visitor& GenericPart::visit(Visitor& v) {
		v.acceptGenericPart(*this);
		return v;
	}

	TypeGeneric::TypeGeneric(ptr<BasicBinding> b, ptr<AndType> t, RelationType rel, VarianceType var1, VarianceType var2, Ast::Location loc)
		: GenericPart{ std::move(b), loc }, rel{ rel }, variadic{ var2 }, variance{ var1 }, impls{ std::move(t) } {}
	Visitor& TypeGeneric::visit(Visitor& v) {
		v.acceptTypeGeneric(*this);
		return v;
	}
	DEF_PRINTER(TypeGeneric) {
		s << std::string(buf, ' ') << context << "ast.TypeGeneric (rel=" << rel._to_string()
		  << ", variance=" << variance._to_string() << ", variadic=" << variadic._to_string() << ')';
		name->prettyPrint(s << '\n', buf + 2, "binding=");
		if (impls && impls->types.size()) impls->prettyPrint(s << '\n', buf + 2, "type=");
		return s;
	}

	ValueGeneric::ValueGeneric(ptr<BasicBinding> b, ptr<Type> t, ptr<ValExpr> v, Ast::Location loc)
		: GenericPart{ std::move(b), loc }, value{ std::move(v) }, type{ std::move(t) } {}
	Visitor& ValueGeneric::visit(Visitor& v) {
		v.acceptValueGeneric(*this);
		return v;
	}
	DEF_PRINTER(ValueGeneric) {
		s << std::string(buf, ' ') << context << "ast.ValueGeneric (type=" << (type != nullptr) << ", val=" << (value != nullptr) << ')';
		name->prettyPrint(s << '\n', buf + 2, "binding=");
		if (value) value->prettyPrint(s << '\n', buf + 2, "def=");
		if (type) type->prettyPrint(s << '\n', buf + 2, "type=");
		return s;
	}

	TypeExt::TypeExt(ptr<Block> b, Ast::Location loc) : Ast{ loc }, cons{}, body{ std::move(b) } {}
	TypeExt::TypeExt(ptr<Tuple> c, ptr<Block> b, Ast::Location loc)
		: Ast{ loc }, cons{ std::move(c) }, body{ std::move(b) } {}
	Visitor& TypeExt::visit(Visitor& v) {
		v.acceptTypeExt(*this);
		return v;
	}
	DEF_PRINTER(TypeExt) {
		s << std::string(buf, ' ') << context << "ast.TypeExt";
		if (cons) cons->prettyPrint(s << '\n', buf + 2, "cons=");
		return body->prettyPrint(s << '\n', buf + 2, "type=");
	}

	Case::Case(ptr<PTuple> vs, ptr<ValExpr> e, ptr<ValExpr> if_g, Ast::Location loc)
		: ValExpr{ loc }, vars{ std::move(vs) }, expr{ std::move(e) }, if_guard{ std::move(if_g) } {}
	Visitor& Case::visit(Visitor& v) {
		v.acceptCase(*this);
		return v;
	}
	DEF_PRINTER(Case) {
		s << std::string(buf, ' ') << context << "ast.Case";
		vars->prettyPrint(s << '\n', buf + 2, "pattern=");
		if (if_guard) if_guard->prettyPrint(s << '\n', buf + 2, "if=");
		return expr->prettyPrint(s << '\n', buf + 2, "expr=");
	}

	ImportPiece::ImportPiece(Ast::Location loc) : Ast{ loc } {}
	Visitor& ImportPiece::visit(Visitor& v) {
		v.acceptImportPiece(*this);
		return v;
	}
	DEF_PRINTER(ImportPiece) {
		return s << std::string(buf, ' ') << context << "ast.ImportAny (_)";
	}

	ImportName::ImportName(ptr<BasicBinding> n, Ast::Location loc)
		: ImportPiece{ loc }, name{ std::move(n) }, new_name{ nullptr }, generic_inst{ nullptr } {}
	Visitor& ImportName::visit(Visitor& v) {
		v.acceptImportName(*this);
		return v;
	}
	DEF_PRINTER(ImportName) {
		s << std::string(buf, ' ') << context << "ast.ImportName (";

		(new_name ? new_name : name)->prettyPrint(s, 0, "name=") << ')';

		if (new_name) {
			name->prettyPrint(s << '\n', buf + 2, "imp=");

			if (generic_inst && generic_inst->elems.size()) {
				s << " [\n";
				for (auto&& e : generic_inst->elems)
					e->prettyPrint(s, buf + 3) << '\n';
				s << std::string(buf + 2, ' ') << ']';
			}
		}

		return s;
	}

	ImportGroup::ImportGroup(std::deque<ptr<ImportPiece>> is, Ast::Location loc) : Sequence{ std::move(is), loc } {}
	Visitor& ImportGroup::visit(Visitor& v) {
		v.acceptImportGroup(*this);
		return v;
	}
	DEF_PRINTER(ImportGroup) {
		s << std::string(buf, ' ') << context << "ast.ImportGroup (size=" << elems.size() << ')';

		for (auto&& imp : elems)
			imp->prettyPrint(s << '\n', buf + 2);

		return s;
	}

	Adt::Adt(ptr<BasicBinding> n, ptr<TupleType> t, Ast::Location loc)
		: Ast{ loc }, name{ std::move(n) }, args{ std::move(t) } {}
	Visitor& Adt::visit(Visitor& v) {
		v.acceptAdt(*this);
		return v;
	}
	DEF_PRINTER(Adt) {
		s << std::string(buf, ' ') << context << "ast.Adt (name=";
		name->prettyPrint(s, 0) << ")";

		if (args) args->prettyPrint(s << '\n', buf + 2, "args=");
		return s;
	}

	Future::Future(bool f, Ast::Location loc) : ValExpr{ loc }, forwarded_from_fn{ f } {}
	Visitor& Future::visit(Visitor& v) {
		v.acceptFuture(*this);
		return v;
	}
	DEF_PRINTER(Future) {
		return s << std::string(buf, ' ') << context << "ast.FutureValue (fwd=" << forwarded_from_fn << ')';
	}

	Branch::Branch(Ast::Location loc) : ValExpr{ loc } {}
	Visitor& Branch::visit(Visitor& v) {
		v.acceptBranch(*this);
		return v;
	}

	Loop::Loop(ptr<ValExpr> b, Ast::Location loc) : Branch{ loc }, body{ std::move(b) } {}
	Visitor& Loop::visit(Visitor& v) {
		v.acceptLoop(*this);
		return v;
	}
	DEF_PRINTER(Loop) {
		s << std::string(buf, ' ') << context << "ast.Loop (";
		ValExpr::prettyPrint(s, buf);
		return body->prettyPrint(s << '\n', buf + 2, "body=");
	}

	While::While(ptr<ValExpr> test, ptr<ValExpr> body, Ast::Location loc)
		: Loop{ std::move(body), loc }, test{ std::move(test) } {}
	Visitor& While::visit(Visitor& v) {
		v.acceptWhile(*this);
		return v;
	}
	DEF_PRINTER(While) {
		s << std::string(buf, ' ') << context << "ast.While (";
		ValExpr::prettyPrint(s, buf);
		test->prettyPrint(s << '\n', buf + 2, "cond=");
		return body->prettyPrint(s << '\n', buf + 2, "body=");
	}

	For::For(ptr<Pattern> p, ptr<ValExpr> g, ptr<ValExpr> b, Ast::Location loc)
		: Loop{ std::move(b), loc }, pattern{ std::move(p) }, generator{ std::move(g) } {}
	Visitor& For::visit(Visitor& v) {
		v.acceptFor(*this);
		return v;
	}
	DEF_PRINTER(For) {
		s << std::string(buf, ' ') << context << "ast.For (";
		ValExpr::prettyPrint(s, buf);

		pattern->prettyPrint(s << '\n', buf + 2, "vars=");
		generator->prettyPrint(s << '\n', buf + 2, "in=");
		return body->prettyPrint(s << '\n', buf + 2, "body=");
	}

	IfElse::IfElse(ptr<ValExpr> test, ptr<ValExpr> body, Ast::Location loc) : Branch{ loc }, else_branch{ nullptr } {
		addBranch(std::move(test), std::move(body));
	}
	void IfElse::addBranch(ptr<ValExpr> test, ptr<ValExpr> body) {
		// TODO: assert else not filled
		if_branches.emplace_back(std::make_pair(std::move(test), std::move(body)));
	}
	Visitor& IfElse::visit(Visitor& v) {
		v.acceptIfElse(*this);
		return v;
	}
	DEF_PRINTER(IfElse) {
		s << std::string(buf, ' ') << context << "ast.Branch (";
		ValExpr::prettyPrint(s, buf);

		size_t cnt = 0;
		for (auto& branch : if_branches) {
			branch.first->prettyPrint(s << '\n', buf + 2, cnt++ ? "elsif=" : "if=");
			branch.second->prettyPrint(s << '\n', buf + 4, "body=");
		}

		if (else_branch) else_branch->prettyPrint(s << '\n', buf + 2, "else=");

		return s;
	}

	Match::Match(ptr<ValExpr> s, std::deque<ptr<Case>> cs, Ast::Location loc)
		: Branch{ loc }, switch_expr{ std::move(s) }, cases{ std::move(cs) } {}
	Visitor& Match::visit(Visitor& v) {
		v.acceptMatch(*this);
		return v;
	}
	DEF_PRINTER(Match) {
		s << std::string(buf, ' ') << context << "ast.Match (";
		ValExpr::prettyPrint(s, buf);

		switch_expr->prettyPrint(s << '\n', buf + 2, "switch=");

		for (auto&& c : cases)
			c->prettyPrint(s << '\n', buf + 2, "case=");

		return s;
	}

	JumpExpr::JumpExpr(ptr<ValExpr> e, Ast::Location loc) : Branch{ loc }, expr{ std::move(e) } {}
	Visitor& JumpExpr::visit(Visitor& v) {
		v.acceptJumpExpr(*this);
		return v;
	}

	Wait::Wait(ptr<ValExpr> e, Ast::Location loc) : JumpExpr{ std::move(e), loc } {}
	Visitor& Wait::visit(Visitor& v) {
		v.acceptWait(*this);
		return v;
	}
	DEF_PRINTER(Wait) {
		s << std::string(buf, ' ') << context << "ast.Wait (";
		ValExpr::prettyPrint(s, buf);

		if (expr) expr->prettyPrint(s << '\n', buf + 2, "expr=");
		return s;
	}

	Break::Break(ptr<ValExpr> e, Ast::Location loc) : JumpExpr{ std::move(e), loc } {}
	Visitor& Break::visit(Visitor& v) {
		v.acceptBreak(*this);
		return v;
	}
	DEF_PRINTER(Break) {
		s << std::string(buf, ' ') << context << "ast.Break (";
		ValExpr::prettyPrint(s, buf);

		if (expr) expr->prettyPrint(s << '\n', buf + 2, "expr=");
		return s;
	}

	Continue::Continue(ptr<ValExpr> e, Ast::Location loc) : JumpExpr{ std::move(e), loc } {}
	Visitor& Continue::visit(Visitor& v) {
		v.acceptContinue(*this);
		return v;
	}
	DEF_PRINTER(Continue) {
		s << std::string(buf, ' ') << context << "ast.Continue (";
		ValExpr::prettyPrint(s, buf);

		if (expr) expr->prettyPrint(s << '\n', buf + 2, "expr=");
		return s;
	}

	Return::Return(ptr<ValExpr> e, Ast::Location loc) : JumpExpr{ std::move(e), loc } {}
	Visitor& Return::visit(Visitor& v) {
		v.acceptReturn(*this);
		return v;
	}
	DEF_PRINTER(Return) {
		s << std::string(buf, ' ') << context << "ast.Return (";
		ValExpr::prettyPrint(s, buf);

		if (expr) expr->prettyPrint(s << '\n', buf + 2, "expr=");
		return s;
	}

	YieldRet::YieldRet(ptr<ValExpr> e, Ast::Location loc) : JumpExpr{ std::move(e), loc } {}
	Visitor& YieldRet::visit(Visitor& v) {
		v.acceptYieldRet(*this);
		return v;
	}
	DEF_PRINTER(YieldRet) {
		s << std::string(buf, ' ') << context << "ast.Yield (";
		ValExpr::prettyPrint(s, buf);

		if (expr) expr->prettyPrint(s << '\n', buf + 2, "expr=");
		return s;
	}

	FnBody::FnBody(ptr<ValExpr> b, bool f, Ast::Location loc) : ValExpr{ loc }, forward{ f }, body{ std::move(b) } {}
	Visitor& FnBody::visit(Visitor& v) {
		v.acceptFnBody(*this);
		return v;
	}
	DEF_PRINTER(FnBody) {
		s << std::string(buf, ' ') << context << "ast.FnBody (fwd=" << (forward ? "true, " : "false, ");
		ValExpr::prettyPrint(s, buf);

		if (ret) ret->prettyPrint(s << '\n', buf + 2, "returns=");
		if (args) args->prettyPrint(s << '\n', buf + 2, "args=");
		return body->prettyPrint(s << '\n', buf + 2, "expr=");
	}

	FnCall::FnCall(ptr<ValExpr> c, ptr<TypeExt> t, ptr<Tuple> a, ptr<Array> i, Ast::Location loc)
		: ValExpr{ loc }, caller{ std::move(c) }, anon{ std::move(t) }, args{ std::move(a) }, inst{ std::move(i) } {}
	Visitor& FnCall::visit(Visitor& v) {
		v.acceptFnCall(*this);
		return v;
	}
	DEF_PRINTER(FnCall) {
		s << std::string(buf, ' ') << context << "ast.FnCall (anon="
			<< (bool)anon << ", args=" << (bool)args << ", inst=" << (bool)inst << ", ";
		ValExpr::prettyPrint(s, buf) << '\n';
		caller->prettyPrint(s, buf + 2, "caller=");

		if (anon) anon->prettyPrint(s << '\n', buf + 2, "anon=");
		if (args) args->prettyPrint(s << '\n', buf + 2, "args=");
		if (inst) inst->prettyPrint(s << '\n', buf + 2, "inst=");
		return s;
	}

	BinOpCall::BinOpCall(ptr<ValExpr> lhs, ptr<ValExpr> rhs, ptr<QualifiedBinding> op, Ast::Location loc)
		: ValExpr{ loc }, lhs{ std::move(lhs) }, rhs{ std::move(rhs) }, op{ std::move(op) } {}
	Visitor& BinOpCall::visit(Visitor& v) {
		v.acceptBinOpCall(*this);
		return v;
	}
	DEF_PRINTER(BinOpCall) {
		s << std::string(buf, ' ') << context << "ast.BinOpCall (";
		ValExpr::prettyPrint(s, buf);
		op->prettyPrint(s << '\n', buf + 2, "op=");
		lhs->prettyPrint(s << '\n', buf + 2, "lhs=");
		return rhs->prettyPrint(s << '\n', buf + 2, "rhs=");
	}

	Range::Range(ptr<ValExpr> start, ptr<ValExpr> stop, Ast::Location loc)
		: ValExpr{ loc }, start{ std::move(start) }, stop{ std::move(stop) } {}
	Visitor& Range::visit(Visitor& v) {
		v.acceptRange(*this);
		return v;
	}
	DEF_PRINTER(Range) {
		s << std::string(buf, ' ') << context << (stop ? "ast.Range (" : "ast.InfRange (");
		ValExpr::prettyPrint(s, buf);

		start->prettyPrint(s << '\n', buf + 2, "start=");
		//if (step) step->prettyPrint(s << '\n', buf + 2, "step=");
		if (stop) stop->prettyPrint(s << '\n', buf + 2, "stop=");
		return s;
	}

	UnaryOpApp::UnaryOpApp(ptr<ValExpr> e, UnaryType t, Ast::Location loc)
		: ValExpr{ loc }, op{ t }, expr{ std::move(e) } {}
	Visitor& UnaryOpApp::visit(Visitor& v) {
		v.acceptUnaryOpApp(*this);
		return v;
	}
	DEF_PRINTER(UnaryOpApp) {
		s << std::string(buf, ' ') << context << "ast.UnaryApp (op=" << op._to_string();
		ValExpr::prettyPrint(s << ", ", buf);

		return expr->prettyPrint(s << '\n', buf + 2, "expr=");
	}

	Interface::Interface(ptr<AssignPattern> n, GenArray gs, ptr<Type> t, Ast::Location loc)
		: Stmt{ loc }, vis{ VisibilityType::PRIVATE }, name{ std::move(n) }, generics{ std::move(gs) }, type{ std::move(t) } {}
	Visitor& Interface::visit(Visitor& v) {
		v.acceptInterface(*this);
		return v;
	}
	DEF_PRINTER(Interface) {
		s << std::string(buf, ' ') << context << "ast.Interface (vis=" << vis._to_string() << ')';
		name->prettyPrint(s << '\n', buf + 2, "var=");

		if (type) type->prettyPrint(s << '\n', buf + 2, "type=");

		if (generics.size()) {
			s << '\n' << std::string(buf + 2, ' ') << "Generic Typing (size=" << generics.size() << ") [";

			for (auto&& g : generics)
				g->prettyPrint(s << '\n', buf + 4);

			s << '\n' << std::string(buf + 2, ' ') << ']';
		}

		return s;
	}

	TypeAssign::TypeAssign(ptr<AssignPattern> n, std::deque<ptr<Ast>> cs, GenArray gs, ptr<Block> b, ptr<Type> t, bool m, Ast::Location loc)
		: Interface{ std::move(n), std::move(gs), std::move(t), loc }, cons{ std::move(cs) }, body{ std::move(b) }, mutable_only{ m } {}
	Visitor& TypeAssign::visit(Visitor& v) {
		v.acceptTypeAssign(*this);
		return v;
	}
	DEF_PRINTER(TypeAssign) {
		s << std::string(buf, ' ') << context << "ast.TypeAssign (vis=" << vis._to_string() << ", mut=" << mutable_only << ')';
		name->prettyPrint(s << '\n', buf + 2, "var=");

		if (type) type->prettyPrint(s << '\n', buf + 2, "type=");

		if (generics.size()) {
			s << '\n' << std::string(buf + 2, ' ') << "Generic Typing (size=" << generics.size() << ") [";

			for (auto&& g : generics)
				g->prettyPrint(s << '\n', buf + 4);

			s << '\n' << std::string(buf + 2, ' ') << ']';
		}

		if (cons.size()) {
			s << '\n' << std::string(buf + 2, ' ') << "Constructor List (size=" << cons.size() << ") [";

			for (auto&& con : cons)
				con->prettyPrint(s << '\n', buf + 4);

			s << '\n' << std::string(buf + 2, ' ') << ']';
		}

		return body->prettyPrint(s << '\n', buf + 2, "body=");
	}

	VarAssign::VarAssign(ptr<AssignPattern> n, GenArray gs, ptr<ValExpr> v, ptr<Type> t, Ast::Location loc)
		: Interface{ std::move(n), std::move(gs), std::move(t), loc }, expr{ std::move(v) } {}
	Visitor& VarAssign::visit(Visitor& v) {
		v.acceptVarAssign(*this);
		return v;
	}
	DEF_PRINTER(VarAssign) {
		s << std::string(buf, ' ') << context << "ast.VarAssign (vis=" << vis._to_string() << ')';
		name->prettyPrint(s << '\n', buf + 2, "var=");
		if (type) type->prettyPrint(s << '\n', buf + 2, "type=");

		if (generics.size()) {
			s << '\n' << std::string(buf + 2, ' ') << "Generic Typing (size=" << generics.size() << ") [";

			for (auto&& g : generics)
				g->prettyPrint(s << '\n', buf + 4);

			s << '\n' << std::string(buf + 2, ' ') << ']';
		}

		return expr->prettyPrint(s << '\n', buf + 2, "value=");
	}

	InAssign::InAssign(ptr<ValExpr> e, Ast::Location loc) : ValExpr{ loc }, expr{ std::move(e) } {}
	Visitor& InAssign::visit(Visitor& v) {
		v.acceptInAssign(*this);
		return v;
	}
	DEF_PRINTER(InAssign) {
		s << std::string(buf, ' ') << context << "ast.ScopedBinding\n";
		binding->prettyPrint(s, buf + 2, "bind=") << '\n';
		return expr->prettyPrint(s, buf + 2, "in=");
	}

	ImplExpr::ImplExpr(ptr<GenericType> t, ptr<Block> b, Ast::Location loc)
		: Stmt{ loc }, type{ std::move(t) }, impls{ std::move(b) } {}
	Visitor& ImplExpr::visit(Visitor& v) {
		v.acceptImplExpr(*this);
		return v;
	}
	DEF_PRINTER(ImplExpr) {
		s << std::string(buf, ' ') << context << "ast.ImplExpr\n";
		type->prettyPrint(s, buf + 2, "type=") << ")\n";
		if (impls) impls->prettyPrint(s, buf + 2, "impl=");
		return Stmt::prettyPrint(s, buf + 2);
	}

	ModDec::ModDec(ptr<QualifiedBinding> v, Ast::Location loc) : Stmt{ loc }, module{ std::move(v) } {}
	Visitor& ModDec::visit(Visitor& v) {
		v.acceptModDec(*this);
		return v;
	}
	DEF_PRINTER(ModDec) {
		s << std::string(buf, ' ') << context << "ast.ModuleDec (module=";
		module->prettyPrint(s, 0) << ')';
		return Stmt::prettyPrint(s, buf + 2);
	}

	ModImport::ModImport(std::deque<ptr<ImportPiece>> ps, Ast::Location loc) : Sequence{ std::move(ps), loc } {}
	Visitor& ModImport::visit(Visitor& v) {
		v.acceptModImport(*this);
		return v;
	}
	DEF_PRINTER(ModImport) {
		s << std::string(buf, ' ') << context << "ast.ModImport";
		Stmt::prettyPrint(s, buf + 2);

		for (auto&& p : elems)
			p->prettyPrint(s << '\n', buf + 2);

		return s;
	}

	Index::Index(ptr<ValExpr> l, ptr<ValExpr> r, Ast::Location loc)
		: Sequence{ MK_DEQUE(std::move(l), std::move(r)), loc } {}
	Visitor& Index::visit(Visitor& v) {
		v.acceptIndex(*this);
		return v;
	}
	DEF_PRINTER(Index) {
		s << std::string(buf, ' ') << context << "ast.IndexSequence (";
		ValExpr::prettyPrint(s, buf);

		for (auto&& e : elems)
			e->prettyPrint(s << '\n', buf + 2, "idx=");

		return s;
	}
}