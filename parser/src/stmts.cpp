//#include "ast/stmts.h"
//#include "ast/atoms.h"
//#include "ast/decor.h"
//#include "ast/names.h"
#include "ast/all.h"

namespace spero::compiler::ast {

	/*
	 * ast::Interface
	 */
	Interface::Interface(ptr<AssignPattern> n, GenArray gs, ptr<Type> t)
		: vis{ VisibilityType::PRIVATE }, name{ std::move(n) }, generics{ std::move(gs) }, type{ std::move(t) } {}
	OutStream& Interface::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.Interface";
	}


	/*
	 * ast::TypeAssign
	 */
	TypeAssign::TypeAssign(ptr<AssignPattern> n, std::deque<ptr<Ast>> cs, GenArray gs, ptr<Block> b, ptr<Type> t)
		: Interface{ std::move(n), std::move(gs), std::move(t) }, cons{ std::move(cs) }, body{ std::move(b) } {}
	OutStream& TypeAssign::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.TypeAssign (vis=" << vis._to_string() << ')';
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


	/*
	 * ast::VarAssign
	 */
	VarAssign::VarAssign(ptr<AssignPattern> n, GenArray gs, ptr<ValExpr> v, ptr<Type> t)
		: Interface{ std::move(n), std::move(gs), std::move(t) }, expr{ std::move(v) } {}
	OutStream& VarAssign::prettyPrint(OutStream& s, size_t buf, std::string context) {
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


	/*
	 * ast::ImplExpr
	 */
	ImplExpr::ImplExpr(ptr<QualifiedBinding> t) : type{ std::move(t) } {}
	OutStream& ImplExpr::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.ImplExpr (type=";
		type->prettyPrint(s, 0) << ')';
		return Stmt::prettyPrint(s, buf + 2);
	}


	/*
	 * ast::ModDec
	 */
	ModDec::ModDec(ptr<QualifiedBinding> v) : module{ std::move(v) } {}
	OutStream& ModDec::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.ModuleDec (module=";
		module->prettyPrint(s, 0) << ')';
		return Stmt::prettyPrint(s, buf + 2);
	}


	/*
	 * ast::ModImport
	 */
	ModImport::ModImport(std::deque<ptr<ImportPiece>> ps) : parts{ std::move(ps) } {}
	OutStream& ModImport::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.ModImport";
		Stmt::prettyPrint(s, buf + 2);

		for (auto&& p : parts)
			p->prettyPrint(s << '\n', buf + 2);

		return s;
	}


	/*
	 * ast::Index
	 */
	Index::Index(ptr<ValExpr> l, ptr<ValExpr> r) {
		elems.push_back(std::move(l));
		elems.push_back(std::move(r));
	}
	OutStream& Index::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.IndexSequence (";
		ValExpr::prettyPrint(s, 0);

		for (auto&& e : elems)
			e->prettyPrint(s << '\n', buf + 2, "idx=");

		return s;
	}

}