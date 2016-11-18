#include "ast.h"
//#include "util/utils.h"

#include <iterator>

//using namespace spero::util;

namespace spero::compiler::ast {
	#define PRETTY_PRINT(typ) std::string typ::pretty_print(size_t buf)

	//
	// Parent Nodes
	//
	Ast::Ast() {}
	PRETTY_PRINT(Ast) {
		return "";
	}

	Token::Token(KeywordType t) : val{ t } {}
	Token::Token(PtrStyling t) : val{ t } {}
	Token::Token(VarianceType t) : val{ t } {}
	Token::Token(RelationType t) : val{ t } {}
	Token::Token(VisibilityType t) : val{ t } {}
	Token::Token(BindingType t) : val{ t } {}
	Token::Token(UnaryType t) : val{ t } {}
	PRETTY_PRINT(Token) {
		if (std::holds_alternative<KeywordType>(val))
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
			return "Empty";
	}

	Stmt::Stmt() {}

	ValExpr::ValExpr() {}
	PRETTY_PRINT(ValExpr) {
		return std::string(buf, ' ') + (is_mut ? "mut " : "");
	}


	//
	// Literals
	//
	Bool::Bool(bool b) : val{ b } {}
	PRETTY_PRINT(Bool) {
		return ValExpr::pretty_print(buf) + "Bool: " + (val ? "true" : "false");
	}

	Byte::Byte(const std::string& num, int base) : val{ std::stoul(num, nullptr, base) } {}
	PRETTY_PRINT(Byte) {
		return ValExpr::pretty_print(buf) + "Byte: " + std::to_string(val);
	}

	Float::Float(const std::string& num) : val{ std::stof(num) } {}
	PRETTY_PRINT(Float) {
		return ValExpr::pretty_print(buf) + "Float: " + std::to_string(val);
	}

	Int::Int(const std::string& num) : val{ std::stoi(num) } {}
	PRETTY_PRINT(Int) {
		return ValExpr::pretty_print(buf) + "Int: " + std::to_string(val);
	}

	String::String(const std::string& str) : val{ str } {}
	PRETTY_PRINT(String) {
		return ValExpr::pretty_print(buf) + "String: " + val;
	}

	Char::Char(char c) : val{ c } {}
	PRETTY_PRINT(Char) {
		return ValExpr::pretty_print(buf) + "Char: " + val;
	}


	//
	// Bindings
	//
	BasicBinding::BasicBinding(std::string n, BindingType t) : name{ n }, type{ t } {}
	PRETTY_PRINT(BasicBinding) {
		return std::string(buf, ' ') + "Binding: " + name + "(" + type._to_string() + ")";
	}

	QualBinding::QualBinding(ptr<BasicBinding> b) {
		add(std::move(b));
	}
	QualBinding::QualBinding(std::deque<ptr<BasicBinding>>& bs) {
		val.swap(bs);
	}
	void QualBinding::add(ptr<BasicBinding> b) {
		val.emplace_back(std::move(b));
	}
	PRETTY_PRINT(QualBinding) {
		std::string ret(buf, ' ');
		ret += "QualBinding";

		for (auto&& b : val)
			ret += "\n" + b->pretty_print(buf + 1);

		return ret;
	}


	//
	// Types
	//
	Type::Type() {}

	BasicType::BasicType(ptr<BasicBinding> name) : BasicType{ std::make_unique<QualBinding>(std::move(name)) } {}
	BasicType::BasicType(ptr<QualBinding> name) : name{ std::move(name) } {}
	PRETTY_PRINT(BasicType) {
		return name->pretty_print(buf);
	}

	InstType::InstType(ptr<QualBinding> name, ptr<Array> inst, PtrStyling ptr)
		: BasicType{ std::move(name) }, inst{ std::move(inst) }, pointer{ ptr } {}
	PRETTY_PRINT(InstType) {
		auto ret = std::string(buf, ' ') + "Instance Type\n" + BasicType::pretty_print(buf + 1);

		switch (pointer) {
			case PtrStyling::POINTER:
				ret += "&"; break;
			case PtrStyling::VIEW:
				ret += "*";
			default: break;
		}

		if (inst)
			for (auto&& node : inst->vals)
				ret += "\n" + node->pretty_print(buf + 2);

		return ret;
	}

	TupleType::TupleType(std::deque<ptr<Type>>& typs) {
		val.swap(typs);
	}
	PRETTY_PRINT(TupleType) {
		auto ret = std::string(buf, ' ') + "Type Tuple";

		for (auto&& typ : val)
			ret += "\n" + typ->pretty_print(buf + 1);

		return ret;
	}

	FuncType::FuncType(ptr<TupleType> args, ptr<Type> ret) : args{ std::move(args) }, ret{ std::move(ret) } {}
	PRETTY_PRINT(FuncType) {
		auto ret = std::string(buf, ' ') + "Function Signature";

		if (args) ret += "\n" + args->pretty_print(buf + 1);
		if (this->ret) ret += "\n" + this->ret->pretty_print(buf + 1);
		return ret;
	}


	//
	// Atoms
	//
	TypeExt::TypeExt(ptr<Tuple> cons, ptr<Block> body) : cons{ std::move(cons) }, body{ std::move(body) } {}
	PRETTY_PRINT(TypeExt) {
		return (cons ? cons->pretty_print(buf) + "\n" : "") + body->pretty_print(buf);
	}

	Sequence::Sequence(std::deque<node>& vals) {
		this->vals.swap(vals);
	}

	Tuple::Tuple(std::deque<node>& vals) : Sequence{ vals } {}
	PRETTY_PRINT(Tuple) {
		std::string ret(buf, ' ');
		ret += "Tuple";

		for (auto&& val : vals)
			ret += "\n" + val->pretty_print(buf + 1);

		return ret;
	}

	Array::Array(std::deque<node>& vals) : Sequence{ vals } {}
	PRETTY_PRINT(Array) {
		std::string ret(buf, ' ');
		ret += "Array";

		for (auto&& val : vals)
			ret += "\n" + val->pretty_print(buf + 1);

		return ret;
	}

	Block::Block(std::deque<node>& vals) : Sequence{ vals } {}
	PRETTY_PRINT(Block) {
		std::string ret(buf, ' ');
		ret += "Block";

		for (auto&& val : vals)
			ret += "\n" + val->pretty_print(buf + 1);

		return ret;
	}

	FnCall::FnCall(node expr) : FnCall(std::move(expr), nullptr, nullptr, nullptr) {}
	FnCall::FnCall(node expr, ptr<TypeExt> anon, ptr<Tuple> args, ptr<Array> inst)
		: caller{ std::move(expr) }, anon{ std::move(anon) }, args{ std::move(args) }, inst{ std::move(inst) } {}
	PRETTY_PRINT(FnCall) {
		std::string buffer(buf++, ' ');
		auto ret = buffer + "FnCall\n";

		if (caller) ret += caller->pretty_print(buf++);
		if (inst) ret += "\n" + buffer + " Instance Array:\n" + inst->pretty_print(buf + 1);
		if (anon) ret += "\n" + buffer + " Anon Extension:\n" + anon->pretty_print(buf);
		if (args) ret += "\n" + buffer + " Argument Tuple:\n" + args->pretty_print(buf + 1);

		return ret;
	}

	Variable::Variable(ptr<QualBinding> name) : var{ std::move(name) } {}
	PRETTY_PRINT(Variable) {
		return std::string(buf, ' ') + "Variable" + var->pretty_print(1);
	}

	FnBody::FnBody(value body, bool fwd) : body{ std::move(body) }, forward{ fwd } {}
	PRETTY_PRINT(FnBody) {
		auto ret = std::string(buf, ' ') + "FnBody" + (forward ? " (forward)" : "");
		
		if (args)
			ret += "\n" + std::string(buf + 1, ' ') + "Args\n" + args->pretty_print(buf + 2);

		if (this->ret)
			ret += "\n" + std::string(buf + 1, ' ') + "Returns\n" + this->ret->pretty_print(buf + 2);

		return ret + "\n" + body->pretty_print(buf + 1);
	}

	Range::Range(value start, value stop) : start{ std::move(start) }, stop{ std::move(stop) } {}
	PRETTY_PRINT(Range) {
		return std::string(buf, ' ') + "Range Expression\n" + start->pretty_print(buf + 1) + "\n" + stop->pretty_print(buf + 1);
	}


	//
	// Decorators
	//
	Annotation::Annotation(ptr<BasicBinding> name, ptr<Tuple> args, bool global)
		: global{ global }, name{ std::move(name) }, args{ std::move(args) } {}
	PRETTY_PRINT(Annotation) {
		auto ret = std::string(buf, ' ') + "Annotation " + name->pretty_print(0) + (global ? "!" : "");

		if (args)
			for (auto&& node : args->vals)
				ret += "\n" + node->pretty_print(buf + 1);

		return ret;
	}

	GenericPart::GenericPart(ptr<BasicBinding> name, ptr<Type> type) : name{ std::move(name) }, type{ std::move(type) } {}

	TypeGeneric::TypeGeneric(ptr<BasicBinding> name, ptr<Type> type, RelationType rel, VarianceType var)
		: GenericPart{ std::move(name), std::move(type) }, rel{ rel }, var{ var } {}
	PRETTY_PRINT(TypeGeneric) {
		auto ret = std::string(buf, ' ') + "TypeGeneric" + name->pretty_print(1);

		switch (var) {
			case VarianceType::COVARIANT: ret += "+";
				break;
			case VarianceType::CONTRAVARIANT: ret += "-";
			default: break;
		}

		if (type)
			ret += "\n" + std::string(buf, ' ') + " Relation (" + rel._to_string() + ")\n" + type->pretty_print(buf + 2);

		return ret;
	}

	ValueGeneric::ValueGeneric(ptr<BasicBinding> name, ptr<Type> type, value val)
		: GenericPart{ std::move(name), std::move(type) }, val{ std::move(val) } {}
	PRETTY_PRINT(ValueGeneric) {
		auto ret = std::string(buf, ' ') + "ValueGeneric\n" + type->pretty_print(buf + 1);
		
		if (val)
			ret += "\n" + std::string(buf + 1, ' ') + "Default Value\n" + val->pretty_print(buf + 2);

		return ret;
	}

	Case::Case(value val) : expr{ std::move(val) } {}
	void Case::setPattern(std::deque<ptr<Pattern>>& vars) {
		this->vars.swap(vars);
	}
	PRETTY_PRINT(Case) {
		auto buffer = std::string(buf, ' ');
		auto ret = buffer + "Case\n" + buffer + " Pattern:\n";

		for (auto&& pat : vars)
			ret += pat->pretty_print(buf + 2) + "\n";

		return ret + expr->pretty_print(buf + 1);
	}

	ImportPart::ImportPart(std::deque<ptr<ImportPiece>>& val) {
		this->val.swap(val);
	}
	void ImportPart::add(ptr<ImportPiece> imp) {
		val.push_back(std::move(imp));
	}
	PRETTY_PRINT(ImportPart) {
		auto ret = std::string(buf, ' ') + "ImportPart";

		for (auto&& node : val)
			ret += "\n" + node->pretty_print(buf + 1);
		
		return ret;
	}

	ImportName::ImportName(ptr<BasicBinding> old) : ImportName{ nullptr, std::move(old) } {}
	ImportName::ImportName(ptr<BasicBinding> old, ptr<BasicBinding> name)
		: old{ std::move(old) }, name{ std::move(name) } {}
	PRETTY_PRINT(ImportName) {
		auto ret = std::string(buf, ' ') + "Import";

		if (old)
			ret += "Rebind\n" + old->pretty_print(buf + 1) + "\n" + name->pretty_print(buf + 1);

		else
			ret += "Named" + name->pretty_print(1);

		return ret;
	}

	ImportGroup::ImportGroup(std::deque<ptr<ImportName>>& group) {
		imps.swap(group);
	}
	PRETTY_PRINT(ImportGroup) {
		auto ret = std::string(buf, ' ') + "ImportGroup";

		for (auto&& node : imps)
			ret += "\n" + node->pretty_print(buf + 1);

		return ret;
	}
	

	//
	// Assignment
	//
	Adt::Adt(ptr<BasicBinding> name, ptr<TupleType> args) : name{ std::move(name) }, args{ std::move(args) } {}
	PRETTY_PRINT(Adt) {
		auto ret = std::string(buf, ' ') + "Adt " + name->pretty_print(0);

		if (args)
			ret += "\n" + args->pretty_print(buf + 1);

		return ret;
	}

	AssignName::AssignName(ptr<ast::BasicBinding> name) : var{ std::move(name) } {}
	PRETTY_PRINT(AssignName) {
		return var->pretty_print(buf);
	}

	AssignTuple::AssignTuple(std::deque<ptr<ast::AssignPattern>>& pat) {
		vars.swap(pat);
	}
	PRETTY_PRINT(AssignTuple) {
		if (vars.size() == 0) return "";

		std::string ret;
		for (size_t i = 0; i < vars.size() - 1; ++i)
			ret += vars[i]->pretty_print(buf) + "\n";

		return ret + vars.back()->pretty_print(buf);
	}

	PRETTY_PRINT(Pattern) {
		return std::string(buf, ' ') + (is_mut ? "mut " : "") + "Anything (_)";
	}

	PTuple::PTuple(std::deque<ptr<Pattern>>& pats) {
		val.swap(pats);
	}
	PRETTY_PRINT(PTuple) {
		std::string ret(buf, ' ');
		ret += (is_mut ? "mut PTuple" : "PTuple");

		for (auto&& pat : val)
			ret += "\n" + pat->pretty_print(buf + 1);

		return ret;
	}

	PNamed::PNamed(ptr<BasicBinding> name) : name{ std::move(name) } {}
	PRETTY_PRINT(PNamed) {
		return std::string(buf, ' ') + (is_mut ? "mut " : "") + "PNamed " + name->pretty_print(0);
	}

	PAdt::PAdt(ptr<BasicBinding> name, ptr<PTuple> arg_decom) : name{ std::move(name) }, args{ std::move(arg_decom) } {}
	PRETTY_PRINT(PAdt) {
		std::string ret(buf, ' ');
		ret += (is_mut ? "mut PAdt " : "PAdt ") + name->pretty_print(0);

		if (args)
			for (auto&& pat : args->val)
				ret += "\n" + args->pretty_print(buf + 1);

		return ret;
	}

	Interface::Interface(ptr<AssignPattern> bind, GenArray& gen, ptr<Type> type)
		: vis{ VisibilityType::PRIVATE }, binding{ std::move(bind) }, type{ std::move(type) } {
		generics.swap(gen);
	}
	void Interface::setVisibility(VisibilityType vis) {
		this->vis = vis;
	}
	PRETTY_PRINT(Interface) {
		return std::string(buf, ' ') + "Interface";
	}

	TypeAssign::TypeAssign(ptr<AssignPattern> bind, std::deque<node>& cons, GenArray& gen, ptr<Block> body, ptr<Type> type)
			: Interface{ std::move(bind), gen, std::move(type) }, body{ std::move(body) } {
		this->cons.swap(cons);
	}
	PRETTY_PRINT(TypeAssign) {
		auto ret = std::string(buf, ' ') + vis._to_string() + " Assignment (Type)\n";
		ret += binding->pretty_print(buf + 1);

		if (cons.size()) {
			ret += "\n" + std::string(buf + 1, ' ') + "Constructors";
			for (auto&& con : cons)
				ret += "\n" + con->pretty_print(buf + 2);
		}

		return ret + "\n" + body->pretty_print(buf + 1);
	}

	VarAssign::VarAssign(ptr<AssignPattern> bind, GenArray& gen, value val, ptr<Type> type)
		: Interface{ std::move(bind), gen, std::move(type) }, expr{ std::move(val) } {}
	PRETTY_PRINT(VarAssign) {
		auto ret = std::string(buf, ' ') + vis._to_string() + " Assignment (Variable)\n";
		ret += binding->pretty_print(buf + 1);
		return ret + "\n" + expr->pretty_print(buf + 1);
	}

	//
	// Control
	//
	Branch::Branch(value tst, value body) : else_branch{ nullptr }	 {
		addIf(std::move(tst), std::move(body));
	}
	void Branch::addIf(value tst, value body) {
		cases.emplace_back(std::make_pair(std::move(tst), std::move(body)));
	}
	void Branch::addElse(value body) {
		else_branch = std::move(body);
	}
	PRETTY_PRINT(Branch) {
		std::string buffer(buf, ' ');
		auto ret = buffer + "IfBranch";

		size_t idx = 0;
		for (auto&& branch : cases) {
			ret += "\n" + buffer + " [" + std::to_string(idx++) + "]:\n"
				+ branch.first->pretty_print(buf + 2) + "\n" + branch.second->pretty_print(buf + 2);
		}

		if (else_branch)
			return ret + "\n" + buffer + " Else\n" + else_branch->pretty_print(buf + 2);

		return ret;
	}

	Loop::Loop(value expr) : body{ std::move(expr) } {}
	PRETTY_PRINT(Loop) {
		return std::string(buf, ' ') + "Loop\n" + body->pretty_print(buf + 1);
	}

	While::While(value test, value body) : test{ std::move(test) }, body{ std::move(body) } {}
	PRETTY_PRINT(While) {
		return std::string(buf, ' ') + "While\n" + test->pretty_print(buf + 1) + "\n" + body->pretty_print(buf + 1);
	}

	For::For(ptr<Pattern> pt, value gen, value body)
		: pattern{ std::move(pt) }, generator{ std::move(gen) }, body{ std::move(body) } {}
	PRETTY_PRINT(For) {
		std::string ret(buf, ' ');
		ret += "For " + pattern->pretty_print(0) + "\n";
		ret += generator->pretty_print(buf + 1) + "\n";
		return ret + body->pretty_print(buf + 1);
	}

	Match::Match(value expr, std::deque<ptr<Case>>& cases) : switch_val{ std::move(expr) } {
		this->cases.swap(cases);
	}
	PRETTY_PRINT(Match) {
		std::string ret(buf, ' ');
		ret += "Match\n" + switch_val->pretty_print(buf + 1);

		for (auto&& c : cases)
			ret += "\n" + c->pretty_print(buf + 1);
		return ret;
	}

	Jump::Jump(KeywordType jmp, value expr) : jmp{ jmp }, body{ std::move(expr) } {}

	Wait::Wait(value expr) : Jump(KeywordType::WAIT, std::move(expr)) {}
	PRETTY_PRINT(Wait) {
		return std::string(buf, ' ') + "Wait\n" + body->pretty_print(buf + 1);
	}

	Break::Break(value expr) : Jump(KeywordType::BREAK, std::move(expr)) {}
	PRETTY_PRINT(Break) {
		return std::string(buf, ' ') + "Break\n" + body->pretty_print(buf + 1);
	}

	Continue::Continue(value expr) : Jump(KeywordType::CONT, std::move(expr)) {}
	PRETTY_PRINT(Continue) {
		return std::string(buf, ' ') + "Continue\n" + body->pretty_print(buf + 1);
	}

	Return::Return(value expr) : Jump(KeywordType::RET, std::move(expr)) {}
	PRETTY_PRINT(Return) {
		return std::string(buf, ' ') + "Return\n" + body->pretty_print(buf + 1);
	}

	YieldExpr::YieldExpr(value expr) : Jump(KeywordType::YIELD, std::move(expr)) {}
	PRETTY_PRINT(YieldExpr) {
		return std::string(buf, ' ') + "Yield\n" + body->pretty_print(buf + 1);
	}


	//
	// Stmts
	//
	ImplExpr::ImplExpr(ptr<QualBinding> type) : type{ std::move(type) } {}
	PRETTY_PRINT(ImplExpr) {
		return std::string(buf, ' ') + "Implements " + type->pretty_print(0);
	}

	ModDec::ModDec(ptr<QualBinding> name) : module{ std::move(name) } {}
	PRETTY_PRINT(ModDec) {
		return std::string(buf, ' ') + "Module " + module->pretty_print(0);
	}

	ModImport::ModImport(std::deque<ptr<ImportPart>>& imps) {
		parts.swap(imps);
	}
	PRETTY_PRINT(ModImport) {
		auto ret = std::string(buf, ' ') + "Import Stmt";

		for (auto&& p : parts) ret += "\n" + p->pretty_print(buf + 1);

		return ret;
	}

	Index::Index(value start, value next) {
		elems.push_back(std::move(start));
		elems.push_back(std::move(next));
	}
	void Index::add(value field) {
		elems.push_back(std::move(field));
	}
	PRETTY_PRINT(Index) {
		auto ret = std::string(buf, ' ') + "Index Sequence";

		for (auto&& elem : elems) ret += "\n" + elem->pretty_print(buf + 1);

		return ret;
	}


	#undef PRETTY_PRINT
}
