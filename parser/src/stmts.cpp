#include "ast/stmts.h"
#include "ast/atoms.h"
#include "ast/decor.h"
#include "ast/names.h"
#include "ast/types.h"

namespace spero::compiler::ast {

	/*
	 * ast::Interface
	 */
	Interface::Interface(ptr<AssignPattern> n, GenArray gs, ptr<Type> t, Ast::Location loc)
		: Stmt{ loc }, vis{ VisibilityType::PRIVATE }, name{ std::move(n) }, generics{ std::move(gs) }, type{ std::move(t) } {}
	OutStream& Interface::prettyPrint(OutStream& s, size_t buf, std::string context) {
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


	/*
	 * ast::TypeAssign
	 */
	TypeAssign::TypeAssign(ptr<AssignPattern> n, std::deque<ptr<Ast>> cs, GenArray gs, ptr<Block> b, ptr<Type> t, Ast::Location loc)
		: Interface{ std::move(n), std::move(gs), std::move(t), loc }, cons{ std::move(cs) }, body{ std::move(b) } {}
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
	VarAssign::VarAssign(ptr<AssignPattern> n, GenArray gs, ptr<ValExpr> v, ptr<Type> t, Ast::Location loc)
		: Interface{ std::move(n), std::move(gs), std::move(t), loc }, expr{ std::move(v) } {}
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
	OutStream& VarAssign::assemblyCode(OutStream& out) {
		return expr->assemblyCode(out);
	}
	
	/*
	 * ast::InAssign
	 */
	InAssign::InAssign(ptr<ValExpr> e, Ast::Location loc) : ValExpr{ loc }, expr{ std::move(e) } {}
	OutStream& InAssign::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.ScopedBinding\n";
		binding->prettyPrint(s, buf + 2, "bind=") << '\n';
		return expr->prettyPrint(s, buf + 2, "in=");
	}

	/*
	 * ast::ImplExpr
	 */
	ImplExpr::ImplExpr(ptr<GenericType> t, ptr<Block> b, Ast::Location loc)
			: Stmt{ loc }, type{ std::move(t) }, impls{ std::move(b) } {}
	OutStream& ImplExpr::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.ImplExpr\n";
		type->prettyPrint(s, buf + 2, "type=") << ")\n";
		if (impls) impls->prettyPrint(s, buf + 2, "impl=");
		return Stmt::prettyPrint(s, buf + 2);
	}


	/*
	 * ast::ModDec
	 */
	ModDec::ModDec(ptr<QualifiedBinding> v, Ast::Location loc) : Stmt{ loc }, module{ std::move(v) } {}
	OutStream& ModDec::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.ModuleDec (module=";
		module->prettyPrint(s, 0) << ')';
		return Stmt::prettyPrint(s, buf + 2);
	}


	/*
	 * ast::ModImport
	 */
	ModImport::ModImport(std::deque<ptr<ImportPiece>> ps, Ast::Location loc) : Stmt{ loc }, parts{ std::move(ps) } {}
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
	Index::Index(ptr<ValExpr> l, ptr<ValExpr> r, Ast::Location loc) : ValExpr{ loc } {
		elems.push_back(std::move(l));
		elems.push_back(std::move(r));
	}
	OutStream& Index::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.IndexSequence (";
		ValExpr::prettyPrint(s, buf);

		for (auto&& e : elems)
			e->prettyPrint(s << '\n', buf + 2, "idx=");

		return s;
	}

}