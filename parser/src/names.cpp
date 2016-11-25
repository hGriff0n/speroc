//#include "ast/names.h"
#include "ast/all.h"

namespace spero::compiler::ast {

	/*
	 * ast::BasicBinding
	 */
	BasicBinding::BasicBinding(std::string n, BindingType t) : name{ std::move(n) }, type{ t } {}
	OutStream& BasicBinding::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.BasicBinding";
	}


	/*
	 * ast::QualifiedBinding
	 */
	QualifiedBinding::QualifiedBinding(ptr<BasicBinding> b) {
		parts.push_back(std::move(b));
	}
	QualifiedBinding::QualifiedBinding(std::deque<ptr<BasicBinding>> ps) : parts{ std::move(ps) } {}
	OutStream& QualifiedBinding::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << parts.front()->name;

		for (auto p = std::begin(parts) + 1; p != std::end(parts); ++p)
			s << ":" << p->get()->name;

		return s;
	}


	/*
	 * ast::Variable
	 */
	Variable::Variable(ptr<QualifiedBinding> n) : name{ std::move(n) } {}
	OutStream& Variable::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.Variable";
	}


	/*
	 * ast::AssignName
	 */
	AssignName::AssignName(ptr<BasicBinding> n) : var{ std::move(n) } {}
	OutStream& AssignName::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.AssignName";
	}


	/*
	 * ast::AssignTuple
	 */
	AssignTuple::AssignTuple(std::deque<ptr<AssignPattern>> vs) : vars{ std::move(vs) } {}
	OutStream& AssignTuple::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.AssignTuple";
	}


	/*
	 * ast::Pattern
	 */
	OutStream& Pattern::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.Pattern";
	}


	/*
	 * ast::PTuple
	 */
	PTuple::PTuple(std::deque<ptr<Pattern>> ps) : ptns{ std::move(ps) } {}
	OutStream& PTuple::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.PTuple";
	}


	/*
	 * ast::PNamed
	 */
	PNamed::PNamed(ptr<BasicBinding> n) : name{ std::move(n) } {}
	OutStream& PNamed::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.PNamed";
	}


	/*
	 * ast::PAdt
	 */
	PAdt::PAdt(ptr<BasicBinding> n, ptr<PTuple> as) : PNamed{ std::move(n) }, args{ std::move(as) } {}
	OutStream& PAdt::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.PAdt";
	}


	/*
	 * ast::PVal
	 */
	PVal::PVal(ptr<ValExpr> v) : value{ std::move(v) } {}
	OutStream& PVal::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.PVal";
	}

}