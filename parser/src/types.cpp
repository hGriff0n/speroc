#include "ast/types.h"
#include "ast/names.h"
#include "ast/atoms.h"
#include "ast/decor.h"

namespace spero::compiler::ast {
	
	/*
	 * ast::Type
	 */
	Type::Type(Ast::Location loc) : Ast{ loc } {}


	/*
	 * ast::BasicType
	 */
	BasicType::BasicType(ptr<BasicBinding> b, PtrStyling p, Ast::Location loc)
		: BasicType{ std::make_unique<ast::QualifiedBinding>(std::move(b), loc), p, loc } {}
	BasicType::BasicType(ptr<QualifiedBinding> b, PtrStyling p, Ast::Location loc)
		: Type{ loc }, name{ std::move(b) }, pointer{ p } {}
	OutStream& BasicType::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.BasicType \"";
		return name->prettyPrint(s, 0) << "\" (ptr=" << pointer._to_string() << ")";
	}


	/*
	 * ast::GenericType
	 */
	GenericType::GenericType(ptr<QualifiedBinding> b, ptr<Array> a, PtrStyling p, Ast::Location loc)
		: BasicType{ std::move(b), p, loc }, inst{ std::move(a) } {}
	OutStream& GenericType::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.GenericType \"";
		name->prettyPrint(s, 0) << "\" (ptr=" << pointer._to_string() << ")";

		if (inst && inst->exprs.size()) {
			s << " [\n";
			for (auto&& e : inst->exprs)
				e->prettyPrint(s, buf + 1) << '\n';
			s << std::string(buf, ' ') << "]";
		}

		return s;
	}


	/*
	 * ast::TupleType
	 */
	TupleType::TupleType(std::deque<ptr<Type>> ts, Ast::Location loc) : Type{ loc }, elems{ std::move(ts) } {}
	OutStream& TupleType::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.TupleType" << elems.size() << "(";

		for (auto&& t : elems)
			t->prettyPrint(s << '\n', buf + 2);

		return s << '\n' << std::string(buf, ' ') << ')';
	}


	/*
	 * ast::FunctionType
	 */
	FunctionType::FunctionType(ptr<TupleType> as, ptr<Type> r, Ast::Location loc)
		: Type{ loc }, args{ std::move(as) }, ret{ std::move(r) } {}
	OutStream& FunctionType::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.FunctionType";

		args->prettyPrint(s << '\n', buf + 2, "args=");
		return ret->prettyPrint(s << '\n', buf + 2, "ret=");
	}

}