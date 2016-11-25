//#include "ast/types.h"
//#include "ast/names.h"
//#include "ast/atoms.h"
#include "ast/all.h"

namespace spero::compiler::ast {
	
	/*
	 * ast::Type
	 */
	Type::Type() {}


	/*
	 * ast::BasicType
	 */
	BasicType::BasicType(ptr<BasicBinding> b, PtrStyling p) : BasicType{ std::make_unique<QualifiedBinding>(std::move(b)), p } {}
	BasicType::BasicType(ptr<QualifiedBinding> b, PtrStyling p) : name{ std::move(b) }, pointer{ p } {}
	OutStream& BasicType::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.BasicType";
	}


	/*
	 * ast::GenericType
	 */
	GenericType::GenericType(ptr<QualifiedBinding> b, ptr<Array> a, PtrStyling p) : BasicType{ std::move(b), p }, inst{ std::move(a) } {}
	OutStream& GenericType::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.GenericType \"";
		name->prettyPrint(s, 0) << "\" (ptr=" << pointer._to_string() << ")";

		if (inst && inst->exprs.size()) {
			s << " [\n";
			for (auto&& e : inst->exprs)
				e->prettyPrint(s, buf + 1) << "\n";
			s << std::string(buf, ' ') << "]";
		}

		return s;
	}


	/*
	 * ast::TupleType
	 */
	TupleType::TupleType(std::deque<ptr<Type>> ts) : elems{ std::move(ts) } {}
	OutStream& TupleType::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.TupleType";
	}


	/*
	 * ast::FunctionType
	 */
	FunctionType::FunctionType(ptr<TupleType> as, ptr<Type> r) : args{ std::move(as) }, ret{ std::move(r) } {}
	OutStream& FunctionType::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.FunctionType";
	}

}