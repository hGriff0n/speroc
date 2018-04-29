#include "parser/AstVisitor.h"
#include "util/ranges.h"

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
	void Ast::accept(AstVisitor& v) {
		v.visitAst(*this);
	}
	DEF_PRINTER(Ast) {
		return s << std::string(buf, ' ') << context << "ast.Ast";
	}

	void Token::accept(AstVisitor& v) {
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
	void Type::accept(AstVisitor& v) {
		v.visitType(*this);
	}

	Statement::Statement(Location loc) : Ast{ loc } {}
	void Statement::accept(AstVisitor& v) {
		v.visitStatement(*this);
	}
	DEF_PRINTER(Statement) {
		for (auto&& a : annots) {
			a->prettyPrint(s << '\n', buf + 2);
		}

		return s;
	}

	ValExpr::ValExpr(Location loc) : Statement{ loc } {}
	void ValExpr::accept(AstVisitor& v) {
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

	Bool::Bool(bool val, Location loc) : Literal{ loc }, val{ val } {}
	void Bool::accept(AstVisitor& v) {
		v.visitBool(*this);
	}
	DEF_PRINTER(Bool) {
		s << std::string(buf, ' ') << context << "ast.Bool (val=" << (val ? "true, " : "false, ");

		return ValExpr::prettyPrint(s, buf);
	}

	Byte::Byte(std::string val, int base, Location loc) : Literal{ loc }, val{ std::stoul(val, nullptr, base) } {}
	void Byte::accept(AstVisitor& v) {
		v.visitByte(*this);
	}
	DEF_PRINTER(Byte) {
		s << std::string(buf, ' ') << context << "ast.Byte (val=" << val << ", ";

		return ValExpr::prettyPrint(s, buf);
	}

	Float::Float(std::string num, Location loc) : Literal{ loc }, val{ std::stof(num) } {}
	void Float::accept(AstVisitor& v) {
		v.visitFloat(*this);
	}
	DEF_PRINTER(Float) {
		s << std::string(buf, ' ') << context << "ast.Float (val=" << val << ", ";

		return ValExpr::prettyPrint(s, buf);
	}

	Int::Int(std::string num, Location loc) : Literal{ loc }, val{ std::stol(num) } {}
	void Int::accept(AstVisitor& v) {
		v.visitInt(*this);
	}
	DEF_PRINTER(Int) {
		s << std::string(buf, ' ') << context << "ast.Int (val=" << val << ", ";

		return ValExpr::prettyPrint(s, buf);
	}

	Char::Char(char c, Location loc) : Literal{ loc }, val{ c } {}
	void Char::accept(AstVisitor& v) {
		v.visitChar(*this);
	}
	DEF_PRINTER(Char) {
		s << std::string(buf, ' ') << context << "ast.Char (val='" << val << "', ";

		return ValExpr::prettyPrint(s, buf);
	}

	String::String(std::string str, Location loc) : ValExpr{ loc }, val{ str } {}
	void String::accept(AstVisitor& v) {
		v.visitString(*this);
	}
	DEF_PRINTER(String) {
		s << std::string(buf, ' ') << context << "ast.String (val=\"" << val << "\", ";

		return ValExpr::prettyPrint(s, buf);
	}

	Future::Future(bool is_comp_generated, Location loc) : ValExpr{ loc }, generated{ is_comp_generated } {}
	void Future::accept(AstVisitor& v) {
		v.visitFuture(*this);
	}
	DEF_PRINTER(Future) {
		return s << std::string(buf, ' ') << context << "ast.FutureValue (gen=" << generated << ')';
	}

	Tuple::Tuple(std::deque<ptr<ValExpr>> vals, Location loc) : Sequence{ std::move(vals), loc } {}
	void Tuple::accept(AstVisitor& v) {
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
	void Array::accept(AstVisitor& v) {
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
	void Block::accept(AstVisitor& v) {
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
	void Function::accept(AstVisitor& v) {
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

	BasicBinding::BasicBinding(std::string str, BindingType type, Location loc)
		: Ast{ loc }, name{ str }, type{ type } {}
	void BasicBinding::accept(AstVisitor& v) {
		v.visitBasicBinding(*this);
	}
	DEF_PRINTER(BasicBinding) {
		return s << std::string(buf, ' ') << context << "ast.BasicBinding (var=" << name << ", type=" << type._to_string() << ")";
	}
	std::string BasicBinding::toString() {
		return name;
	}


	PathPart::PathPart(std::string str, BindingType type, Location loc)
		: Ast{ loc }, name{ str }, type{ type } {}
	void PathPart::accept(AstVisitor& v) {
		//v.visitBasicBinding(*this);
	}
	DEF_PRINTER(PathPart) {
		s << name;
		if (ssa_id) s << '$' << *ssa_id;
		if (gens) s << "[_]";
		return s;
		//return s << std::string(buf, ' ') << context << "ast.PathPart (var=" << name << ", type=" << type._to_string() << ")";
	}

	Path::Path(ptr<PathPart> fst, Location loc) : Sequence{ MK_DEQUE(std::move(fst)), loc } {}
	void Path::accept(AstVisitor& v) {
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
	void Pattern::accept(AstVisitor& v) {
		v.visitPattern(*this);
	}
	DEF_PRINTER(Pattern) {
		return s << std::string(buf, ' ') << context << "ast.AnyPattern (_)";
	}

	TuplePattern::TuplePattern(std::deque<ptr<Pattern>> parts, Location loc)
		: Sequence{ std::move(parts), loc } {}
	void TuplePattern::accept(AstVisitor& v) {
		v.visitTuplePattern(*this);
	}
	DEF_PRINTER(TuplePattern) {
		s << std::string(buf, ' ') << context << "ast.TuplePattern" << elems.size() << " (capture=" << cap._to_string() << ") (\n";
		for (auto&& p : elems) {
			p->prettyPrint(s, buf + 2) << '\n';
		}
		return s << std::string(buf, ' ') << ')';
	}

	VarPattern::VarPattern(ptr<Path> var, Location loc) : Pattern{ loc }, name{ std::move(var) } {}
	void VarPattern::accept(AstVisitor& v) {
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

	AdtPattern::AdtPattern(ptr<Path> adt_name, ptr<TuplePattern> extract_pattern, Location loc)
		: VarPattern{ std::move(adt_name), loc }, args{ std::move(extract_pattern) } {}
	void AdtPattern::accept(AstVisitor& v) {
		v.visitAdtPattern(*this);
	}
	DEF_PRINTER(AdtPattern) {
		s << std::string(buf, ' ') << context << "ast.AdtPattern (capture=" << cap._to_string();
		name->prettyPrint(s, 0, ", type=") << ')';
		if (args) {
			args->prettyPrint(s << '\n', buf + 2);
		}
		return s;
	}

	ValPattern::ValPattern(ptr<ValExpr> value, Location loc) : Pattern{ loc }, val{ std::move(value) } {}
	void ValPattern::accept(AstVisitor& v) {
		v.visitValPattern(*this);
	}
	DEF_PRINTER(ValPattern) {
		s << std::string(buf, ' ') << context << "ast.PVal";
		return val->prettyPrint(s, 1, "(val=") << ')';
	}

	AssignPattern::AssignPattern(Location loc) : Ast{ loc } {}
	void AssignPattern::accept(AstVisitor& v) {
		v.visitAssignPattern(*this);
	}
	DEF_PRINTER(AssignPattern) {
		return s << std::string(buf, ' ') << context << "ast.AnyPattern (_)";
	}

	AssignName::AssignName(ptr<BasicBinding> name, Location loc) : AssignPattern{ loc }, var{ std::move(name) } {}
	void AssignName::accept(AstVisitor& v) {
		v.visitAssignName(*this);
	}
	DEF_PRINTER(AssignName) {
		return var->prettyPrint(s, buf, context);
	}

	AssignTuple::AssignTuple(std::deque<ptr<AssignPattern>> patterns, Location loc) : Sequence{ std::move(patterns), loc } {}
	void AssignTuple::accept(AstVisitor& v) {
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
	SourceType::SourceType(ptr<Path> binding, Location loc)
		: Type{ loc }, name{ std::move(binding) }, _ptr{ PtrStyling::NA } {}
	void SourceType::accept(AstVisitor& v) {
		//v.visitSourceType(*this);
	}
	DEF_PRINTER(SourceType) {
		s << std::string(buf, ' ') << context << "ast.SourceType \"";
		return name->prettyPrint(s, 0) << "\" (ptr=" << _ptr._to_string() << ")";
	}

	GenericType::GenericType(ptr<Path> binding, ptr<Array> gen_args, Location loc)
		: SourceType{ std::move(binding), loc }, inst{ std::move(gen_args) } {}
	void GenericType::accept(AstVisitor& v) {
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

	TupleType::TupleType(std::deque<ptr<Type>> types, Location loc) : Sequence{ std::move(types), loc } {}
	void TupleType::accept(AstVisitor& v) {
		v.visitTupleType(*this);
	}
	DEF_PRINTER(TupleType) {
		s << std::string(buf, ' ') << context << "ast.TupleType" << elems.size() << "(";

		for (auto&& t : elems) {
			t->prettyPrint(s << '\n', buf + 2);
		}

		return s << '\n' << std::string(buf, ' ') << ')';
	}

	FunctionType::FunctionType(ptr<TupleType> arg_types, ptr<Type> ret_type, Location loc)
		: Type{ loc }, args{ std::move(arg_types) }, ret{ std::move(ret_type) } {}
	void FunctionType::accept(AstVisitor& v) {
		v.visitFunctionType(*this);
	}
	DEF_PRINTER(FunctionType) {
		s << std::string(buf, ' ') << context << "ast.FunctionType";

		args->prettyPrint(s << '\n', buf + 2, "args=");
		if (ret) {
			ret->prettyPrint(s << '\n', buf + 2, "ret=");
		} else {
			s << '\n' << std::string(buf + 2, ' ') << "ret=<error: missing>";
		}
		
		return s;
	}

	AndType::AndType(ptr<Type> typs, Location loc)
		: Sequence{ MK_DEQUE<Type>(std::move(typs)), loc } {}
	void AndType::accept(AstVisitor& v) {
		v.visitAndType(*this);
	}
	DEF_PRINTER(AndType) {
		s << std::string(buf, ' ') << context << "ast.AndType";

		for (auto&& t : elems) {
			if (t) {
				t->prettyPrint(s << '\n', buf + 2);
			} else {
				s << '\n' << std::string(buf + 2, ' ') << "<error: missing>";
			}
		}

		return s;
	}

	OrType::OrType(ptr<Type> typs, Location loc)
		: Sequence{ MK_DEQUE<Type>(std::move(typs)), loc } {}
	void OrType::accept(AstVisitor& v) {
		v.visitOrType(*this);
	}
	DEF_PRINTER(OrType) {
		s << std::string(buf, ' ') << context << "ast.OrType";

		for (auto&& t : elems) {
			if (t) {
				t->prettyPrint(s << '\n', buf + 2);
			} else {
				s << '\n' << std::string(buf + 2, ' ') << "<error: missing>";
			}
		}

		return s;
	}


	//
	// DECORATIONS, ANNOTATION, GENERIC
	//

	Annotation::Annotation(ptr<BasicBinding> annot_name, ptr<Tuple> args, Location loc)
		: Ast{ loc }, name{ std::move(annot_name) }, args{ std::move(args) } {}
	void Annotation::accept(AstVisitor& v) {
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

	LocalAnnotation::LocalAnnotation(ptr<BasicBinding> annot_name, ptr<Tuple> args, Location loc)
		: Annotation{ std::move(annot_name), std::move(args), loc } {}
	void LocalAnnotation::accept(AstVisitor& v) {
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

	GenericPart::GenericPart(ptr<BasicBinding> binding, ptr<Type> typ, RelationType relation, Location loc)
		: Ast{ loc }, type{ std::move(typ) }, name{ std::move(binding) }, rel{ relation } {}
	void GenericPart::accept(AstVisitor& v) {
		v.visitGenericPart(*this);
	}

	TypeGeneric::TypeGeneric(ptr<BasicBinding> symbol, ptr<Type> def, Location loc)
		: TypeGeneric{ std::move(symbol), nullptr, RelationType::NA, VarianceType::INVARIANT, false, loc } { default_type = std::move(def); }
	TypeGeneric::TypeGeneric(ptr<BasicBinding> symbol, ptr<Type> typ, RelationType relation, VarianceType variance_kind, bool is_variadic, Location loc)
		: GenericPart{ std::move(symbol), std::move(typ), relation, loc }, variadic{ is_variadic }, variance{ variance_kind }, default_type{ nullptr } {}
	void TypeGeneric::accept(AstVisitor& v) {
		v.visitTypeGeneric(*this);
	}
	DEF_PRINTER(TypeGeneric) {
		s << std::string(buf, ' ') << context << "ast.TypeGeneric (rel=" << rel._to_string()
			<< ", variance=" << variance._to_string() << ", variadic=" << variadic << ')';
		name->prettyPrint(s << '\n', buf + 2, "binding=");
		if (type) {
			type->prettyPrint(s << '\n', buf + 2, "type=");
		}
		if (default_type) {
			default_type->prettyPrint(s << '\n', buf + 2, "default=");
		}
		return s;
	}

	ValueGeneric::ValueGeneric(ptr<BasicBinding> symbol, ptr<ValExpr> def, Location loc)
		: GenericPart{ std::move(symbol), nullptr, RelationType::NA, loc }, default_val{ std::move(def) } {}
	ValueGeneric::ValueGeneric(ptr<BasicBinding> symbol, ptr<Type> typ, RelationType relation, Location loc)
		: GenericPart{ std::move(symbol), std::move(typ), relation, loc }, default_val{ nullptr } {}
	void ValueGeneric::accept(AstVisitor& v) {
		v.visitValueGeneric(*this);
	}
	DEF_PRINTER(ValueGeneric) {
		s << std::string(buf, ' ') << context << "ast.ValueGeneric (rel=" << rel._to_string() << ')';
		name->prettyPrint(s << '\n', buf + 2, "binding=");
		if (type) {
			type->prettyPrint(s << '\n', buf + 2, "type=");
		}
		if (default_val) {
			default_val->prettyPrint(s << '\n', buf + 2, "default=");
		}
		return s;
	}

	LitGeneric::LitGeneric(ptr<ValExpr> val, Location loc)
		: GenericPart{ nullptr, nullptr, RelationType::NA, loc }, value{ std::move(val) } {}
	void LitGeneric::accept(AstVisitor& v) {
		v.visitLitGeneric(*this);
	}
	DEF_PRINTER(LitGeneric) {
		s << std::string(buf, ' ') << context << "ast.LitGeneric";
		return value->prettyPrint(s << '\n', buf + 2);
	}

	GenericArray::GenericArray(std::deque<ptr<GenericPart>> elems, Location loc)
		: Sequence{ std::move(elems), loc } {}
	void GenericArray::accept(AstVisitor& v) {
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

	Adt::Adt(ptr<BasicBinding> type_name, ptr<TupleType> args, Location loc)
		: Constructor{ loc }, name{ std::move(type_name) }, args{ std::move(args) } {}
	void Adt::accept(AstVisitor& v) {
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

	Argument::Argument(ptr<BasicBinding> binding, ptr<Type> type, Location loc)
		: Ast{ loc }, name{ std::move(binding) }, typ{ std::move(type) } {}
	void Argument::accept(AstVisitor& v) {
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
	void ArgTuple::accept(AstVisitor& v) {
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
	void Branch::accept(AstVisitor& v) {
		v.visitBranch(*this);
	}

	Loop::Loop(ptr<ValExpr> body, Location loc) : Branch{ loc }, body{ std::move(body) } {}
	void Loop::accept(AstVisitor& v) {
		v.visitLoop(*this);
	}
	DEF_PRINTER(Loop) {
		s << std::string(buf, ' ') << context << "ast.Loop (";
		ValExpr::prettyPrint(s, buf);
		return body->prettyPrint(s << '\n', buf + 2, "body=");
	}

	While::While(ptr<ValExpr> test, ptr<ValExpr> body, Location loc)
		: Loop{ std::move(body), loc }, test{ std::move(test) } {}
	void While::accept(AstVisitor& v) {
		v.visitWhile(*this);
	}
	DEF_PRINTER(While) {
		s << std::string(buf, ' ') << context << "ast.While (";
		ValExpr::prettyPrint(s, buf);
		test->prettyPrint(s << '\n', buf + 2, "cond=");
		return body->prettyPrint(s << '\n', buf + 2, "body=");
	}

	For::For(ptr<Pattern> pat, ptr<ValExpr> gen, ptr<ValExpr> body, Location loc)
		: Loop{ std::move(body), loc }, pattern{ std::move(pat) }, generator{ std::move(gen) } {}
	void For::accept(AstVisitor& v) {
		v.visitFor(*this);
	}
	DEF_PRINTER(For) {
		s << std::string(buf, ' ') << context << "ast.For (";
		ValExpr::prettyPrint(s, buf);

		pattern->prettyPrint(s << '\n', buf + 2, "vars=");
		generator->prettyPrint(s << '\n', buf + 2, "in=");
		return body->prettyPrint(s << '\n', buf + 2, "body=");
	}

	IfBranch::IfBranch(ptr<ValExpr> test, ptr<ValExpr> body, bool is_elsif, Location loc)
		: Branch{ loc }, test{ std::move(test) }, body{ std::move(body) }, elsif{ is_elsif } {}
	void IfBranch::accept(AstVisitor& v) {
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
	void IfElse::accept(AstVisitor& v) {
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

	Case::Case(ptr<TuplePattern> extraction_vars, ptr<ValExpr> guard, ptr<ValExpr> expr, Location loc)
		: ValExpr{ loc }, vars{ std::move(extraction_vars) }, expr{ std::move(expr) }, if_guard{ std::move(guard) } {}
	void Case::accept(AstVisitor& v) {
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

	Match::Match(ptr<ValExpr> test, std::deque<ptr<Case>> cases, Location loc)
		: Branch{ loc }, switch_expr{ std::move(test) }, cases{ std::move(cases) } {}
	void Match::accept(AstVisitor& v) {
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

	Jump::Jump(KeywordType kind, ptr<ValExpr> expr, Location loc)
		: Branch{ loc }, type{ kind }, expr{ std::move(expr) } {}
	void Jump::accept(AstVisitor& v) {
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

	ModDec::ModDec(ptr<Path> path, Location loc) : Statement{ loc }, _module{ std::move(path) } {}
	void ModDec::accept(AstVisitor& v) {
		v.visitModDec(*this);
	}
	DEF_PRINTER(ModDec) {
		s << std::string(buf, ' ') << context << "ast.ModuleDec (module=";
		_module->prettyPrint(s, 0) << ')';
		return Statement::prettyPrint(s, buf + 2);
	}

	ImplExpr::ImplExpr(ptr<SourceType> import_type, ptr<SourceType> impl_type, ptr<Block> impls_body, Location loc)
		: Statement{ loc }, type{ std::move(import_type) }, _interface{ std::move(impl_type) }, impls{ std::move(impls_body) } {}
	void ImplExpr::accept(AstVisitor& v) {
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
	ModRebindImport::ModRebindImport(ptr<Path> mod, Location loc) : Statement{ loc }, _module{ std::move(mod) } {}
	DEF_PRINTER(ModRebindImport) {
		if (_module) {
			_module->prettyPrint(s, buf, context);
		} else {
			s << std::string(buf, ' ') << context << "{scope}";
		}

		return s;
	}

	SingleImport::SingleImport(ptr<Path> mod, Location loc) : ModRebindImport{ std::move(mod), loc } {}
	void SingleImport::accept(AstVisitor& v) {
		v.visitSingleImport(*this);
	}
	DEF_PRINTER(SingleImport) {
		s << std::string(buf, ' ') << context << "ast.SingleImport ";
		return ModRebindImport::prettyPrint(s, 0);
	}

	MultipleImport::MultipleImport(ptr<Path> mod, std::deque<ptr<PathPart>> names, Location loc) : Sequence{ std::move(names), loc } {
		_module = std::move(mod);
	}
	void MultipleImport::accept(AstVisitor& v) {
		v.visitMultipleImport(*this);
	}
	DEF_PRINTER(MultipleImport) {
		s << std::string(buf, ' ') << context << "ast.MultipleImport";
		Statement::prettyPrint(s, buf + 2);
		ModRebindImport::prettyPrint(s << '\n', buf + 2, "from=");

		s << '\n' << std::string(buf + 2, ' ') << "import= {";

		for (auto&& name : elems) {
			name->prettyPrint(s, 0) << ", ";
		}

		return s << '}';
	}

	Rebind::Rebind(ptr<Path> mod, ptr<Path> new_binding, Location loc)
		: ModRebindImport{ std::move(mod), loc }, new_name{ std::move(new_binding) } {}
	void Rebind::accept(AstVisitor& v) {
		v.visitRebind(*this);
	}
	DEF_PRINTER(Rebind) {
		s << std::string(buf, ' ') << context << "ast.Rebind";
		Statement::prettyPrint(s, buf + 2);
		ModRebindImport::prettyPrint(s << '\n', buf + 2, "from=");

		new_name->prettyPrint(s << '\n', buf + 2, "to=");

		return s;
	}

	Interface::Interface(VisibilityType vis, ptr<AssignPattern> binding, ptr<GenericArray> gen_pattern, ptr<Type> type_bound, Location loc)
		: Statement{ loc }, vis{ vis }, name{ std::move(binding) }, gen{ std::move(gen_pattern) }, type{ std::move(type_bound) } {}
	void Interface::accept(AstVisitor& v) {
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

	TypeAssign::TypeAssign(VisibilityType vis, ptr<AssignPattern> binding, ptr<GenericArray> gen_pattern, bool mut_only, ConsList constructors, ptr<Block> def, Location loc)
		: Interface{ vis, std::move(binding), std::move(gen_pattern), nullptr, loc }, cons{ std::move(constructors) }, body{ std::move(def) }, mutable_only{ mut_only } {}
	void TypeAssign::accept(AstVisitor& v) {
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

	VarAssign::VarAssign(VisibilityType vis, ptr<AssignPattern> binding, ptr<GenericArray> gen_pattern, ptr<Type> type_bound, ptr<ValExpr> value, Location loc)
		: Interface{ vis, std::move(binding), std::move(gen_pattern), std::move(type_bound), loc }, expr{ std::move(value) } {}
	void VarAssign::accept(AstVisitor& v) {
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

	TypeExtension::TypeExtension(ptr<Path> base_type, ptr<Tuple> base_args, ptr<Block> ext_def, Location loc)
		: ValExpr{ loc }, typ_name{ std::move(base_type) }, args{ std::move(base_args) }, ext{ std::move(ext_def) } {}
	void TypeExtension::accept(AstVisitor& v) {
		v.visitTypeExtension(*this);
	}
	DEF_PRINTER(TypeExtension) {
		s << std::string(buf, ' ') << context << "ast.AnonymousType";
		typ_name->prettyPrint(s << '\n', buf + 2, "extend=");

		if (args) {
			args->prettyPrint(s << '\n', buf + 4);
		}

		return ext->prettyPrint(s << '\n', buf + 2, "with=");
	}


	//
	// EXPRESSIONS
	//

	InAssign::InAssign(ptr<VarAssign> binding, ptr<ValExpr> expr, Location loc)
		: ValExpr{ loc }, bind{ std::move(binding) }, expr{ std::move(expr) } {}
	void InAssign::accept(AstVisitor& v) {
		v.visitInAssign(*this);
	}
	DEF_PRINTER(InAssign) {
		s << std::string(buf, ' ') << context << "ast.ScopedBinding\n";
		bind->prettyPrint(s, buf + 2, "bind=") << '\n';
		return expr->prettyPrint(s, buf + 2, "in=");
	}

	Variable::Variable(ptr<Path> symbol, Location loc)
		: ValExpr{ loc }, name{ std::move(symbol) } {}
	void Variable::accept(AstVisitor& v) {
		v.visitVariable(*this);
	}
	DEF_PRINTER(Variable) {
		s << std::string(buf, ' ') << context << "ast.Variable (";
		ValExpr::prettyPrint(s, buf);
		return name->prettyPrint(s << "\n", buf + 2, "name=");
	}

	/*UnOpCall::UnOpCall(ptr<ValExpr> e, std::string op, Location loc)
		: ValExpr{ loc }, expr{ std::move(e) }, rhs{ std::move(rhs) }, op{ op } {}*/
	UnOpCall::UnOpCall(ptr<ValExpr> expr, ptr<BasicBinding> op, Location loc)
		: ValExpr{ loc }, expr{ std::move(expr) }, op{ std::move(op) } {}
	void UnOpCall::accept(AstVisitor& v) {
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
	void BinOpCall::accept(AstVisitor& v) {
		v.visitBinOpCall(*this);
	}
	DEF_PRINTER(BinOpCall) {
		s << std::string(buf, ' ') << context << "ast.BinOpCall (op=" << op;
		//op->prettyPrint(s << std::string(buf, ' ') << context << "ast.BinOpCall (op=", 0);
		ValExpr::prettyPrint(s << ", ", buf);
		lhs->prettyPrint(s << '\n', buf + 2, "lhs=");
		return rhs->prettyPrint(s << '\n', buf + 2, "rhs=");
	}

	Index::Index(std::deque<ptr<ValExpr>> indices, Location loc)
		: Sequence{ std::move(indices), loc } {}
	void Index::accept(AstVisitor& v) {
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

	FnCall::FnCall(ptr<ValExpr> calling_fn, ptr<Tuple> fn_args, Location loc)
		: ValExpr{ loc }, callee{ std::move(calling_fn) }, arguments{ std::move(fn_args) } {}
	void FnCall::accept(AstVisitor& v) {
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

	TypeError::TypeError(Location loc) : SourceType{ nullptr, loc } {}
	DEF_PRINTER(TypeError) {
		return s << std::string(buf, ' ') << context << "ast.Error";
	}

	ScopeError::ScopeError(Location loc) : Block{ {}, loc } {}
	DEF_PRINTER(ScopeError) {
		return s << std::string(buf, ' ') << context << "ast.Error";
	}

}

namespace spero::compiler::analysis {

	using LookupType = opt_t<SymTable::DataType>;
	std::tuple<LookupType, ast::Path::iterator> lookup(SymTable& globals, SymTable* current, ast::Path& var_path) {
		auto[front, end] = util::range(var_path.elems);
		LookupType value = std::nullopt;
		bool has_next = true;

		// Support forced global indexing (through ':<>') (NOTE: Undecided on inclusion in final document)
		if (!(**front).name.empty()) {
			auto* next = current->mostRecentDef((**front).name);
			value = std::get<ref_t<SymTable>>((*(next ? next : current))["self"].get());
			has_next = next != nullptr;

		} else {
			value = std::get<ref_t<SymTable>>(globals["self"].get());
			++front;
		}

		// Follow the symbol path to it's end
		while (front != end && has_next) {
			auto next = std::visit([&](auto&& var) -> LookupType {
				if constexpr (std::is_same_v<std::decay_t<decltype(var)>, ref_t<SymTable>>) {
					return var.get().ssaIndex((**front).name, (**front).ssa_id, var_path.loc);
				}

				return std::nullopt;
			}, *value);

			if (has_next = next.has_value()) {
				value.swap(next);
				++front;
			}
		}

		// Return the last accessed value and the last attempted symbol if lookup fails
		// This should simplify the process of assigning new variables, etc.
		return { value, front };
	}

	// If Ssa lookup fails, then the key exists but the ssa_id was never set
	bool testSsaLookupFailure(LookupType& lookup_result, ast::Path::iterator& iter) {
		if (lookup_result) {
			if (auto* table = std::get_if<ref_t<SymTable>>(&*lookup_result)) {
				return table->get().exists((**iter).name) && !(**iter).ssa_id;
			}
		}

		return true;
	}
}