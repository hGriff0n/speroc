#include "ast/names.h"
#include "ast/decor.h"
#include "ast/atoms.h"

namespace spero::compiler::ast {

	/*
	 * ast::BasicBinding
	 */
	BasicBinding::BasicBinding(std::string n, BindingType t, Ast::Location loc) : Ast{ loc }, name{ std::move(n) }, type{ t } {}
	OutStream& BasicBinding::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.BasicBinding (var=" << name << ", type=" << type._to_string() << ")";
	}


	/*
	 * ast::QualifiedBinding
	 */
	QualifiedBinding::QualifiedBinding(ptr<BasicBinding> b, Ast::Location loc) : Ast{ loc } {
		parts.push_back(std::move(b));
	}
	QualifiedBinding::QualifiedBinding(std::deque<ptr<BasicBinding>> ps, Ast::Location loc) : Ast{ loc }, parts{ std::move(ps) } {}
	OutStream& QualifiedBinding::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.QualifiedBinding " << parts.front()->name;

		for (auto p = std::begin(parts) + 1; p != std::end(parts); ++p)
			s << ":" << p->get()->name;

		return s;
	}


	/*
	 * ast::Variable
	 */
	Variable::Variable(ptr<QualifiedBinding> n, Ast::Location loc) : ValExpr{ loc }, name{ std::move(n) } {}
	OutStream& Variable::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Variable (";
		ValExpr::prettyPrint(s, buf);
		return name->prettyPrint(s << "\n", buf + 2, "name=");
	}


	/*
	 * ast::AssignPattern
	 */
	AssignPattern::AssignPattern(Ast::Location loc) : Ast{ loc } {}


	/*
	 * ast::AssignName
	 */
	AssignName::AssignName(ptr<BasicBinding> n, Ast::Location loc) : AssignPattern{ loc }, var{ std::move(n) } {}
	OutStream& AssignName::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return var->prettyPrint(s, buf, context);
	}


	/*
	 * ast::AssignTuple
	 */
	AssignTuple::AssignTuple(std::deque<ptr<AssignPattern>> vs, Ast::Location loc) : AssignPattern{ loc }, vars{ std::move(vs) } {}
	OutStream& AssignTuple::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.AssignTuple" << vars.size() << "(\n";
		for (auto&& var : vars)
			var->prettyPrint(s, buf + 2) << '\n';
		return s << std::string(buf, ' ') << ')';
	}


	/*
	 * ast::Pattern
	 */
	Pattern::Pattern(Ast::Location loc) : Ast{ loc } {}
	OutStream& Pattern::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.PatternAny (_)";
	}


	/*
	 * ast::PTuple
	 */
	PTuple::PTuple(std::deque<ptr<Pattern>> ps, Ast::Location loc) : Pattern{ loc }, ptns{ std::move(ps) } {}
	OutStream& PTuple::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.PTuple" << ptns.size() << " (capture=" << cap._to_string() << ") (\n";
		for (auto&& p : ptns)
			p->prettyPrint(s, buf + 2) << '\n';
		return s << std::string(buf, ' ') << ')';
	}


	/*
	 * ast::PNamed
	 */
	PNamed::PNamed(ptr<BasicBinding> n, Ast::Location loc) : Pattern{ loc }, name{ std::move(n) } {}
	OutStream& PNamed::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.PNamed (capture=" << cap._to_string() << ')';
		return name->prettyPrint(s, 1);
	}


	/*
	 * ast::PAdt
	 */
	PAdt::PAdt(ptr<BasicBinding> n, ptr<PTuple> as, Ast::Location loc) : PNamed{ std::move(n), loc }, args{ std::move(as) } {}
	OutStream& PAdt::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.PAdt";
		name->prettyPrint(s, 1, "(type=") << ')';
		if (args) args->prettyPrint(s << '\n', buf + 2);
		return s;
	}


	/*
	 * ast::PVal
	 */
	PVal::PVal(ptr<ValExpr> v, Ast::Location loc) : Pattern{ loc }, value { std::move(v) } {}
	OutStream& PVal::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.PVal";
		return value->prettyPrint(s, 1, "(val=") << ')';
	}

}