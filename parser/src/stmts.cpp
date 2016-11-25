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
	OutStream Interface::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::TypeAssign
	 */
	TypeAssign::TypeAssign(ptr<AssignPattern> n, std::deque<ptr<Ast>> cs, GenArray gs, ptr<Block> b, ptr<Type> t)
		: Interface{ std::move(n), std::move(gs), std::move(t) }, cons{ std::move(cs) }, body{ std::move(b) } {}
	OutStream TypeAssign::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::VarAssign
	 */
	VarAssign::VarAssign(ptr<AssignPattern> n, GenArray gs, ptr<ValExpr> v, ptr<Type> t)
		: Interface{ std::move(n), std::move(gs), std::move(t) }, expr{ std::move(v) } {}
	OutStream VarAssign::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::ImplExpr
	 */
	ImplExpr::ImplExpr(ptr<QualifiedBinding> t) : type{ std::move(t) } {}
	OutStream ImplExpr::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::ModDec
	 */
	ModDec::ModDec(ptr<QualifiedBinding> v) : module{ std::move(v) } {}
	OutStream ModDec::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::ModImport
	 */
	ModImport::ModImport(std::deque<ptr<ImportPiece>> ps) : parts{ std::move(ps) } {}
	OutStream ModImport::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::Index
	 */
	Index::Index(ptr<ValExpr> l, ptr<ValExpr> r) {
		elems.push_back(std::move(l));
		elems.push_back(std::move(r));
	}
	OutStream Index::prettyPrint(OutStream s, size_t buf) {
		return s;
	}

}