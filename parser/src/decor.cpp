#include "ast/decor.h"
#include "ast/atoms.h"
#include "ast/names.h"
#include "ast/types.h"

namespace spero::compiler::ast {

	/*
	 * ast::GlobalAnnotation
	 */
	GlobalAnnotation::GlobalAnnotation(ptr<BasicBinding> n, ptr<Tuple> t) : name{ std::move(n) }, args{ std::move(t) } {}
	OutStream& GlobalAnnotation::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.GlobalAnnotation name=";
		name->prettyPrint(s, 0);

		if (args) args->prettyPrint(s << '\n', buf + 2, "args=");
		return s;
	}


	/*
	 * ast::Annotation
	 */
	Annotation::Annotation(ptr<BasicBinding> n, ptr<Tuple> t) : name{ std::move(n) }, args{ std::move(t) } {}
	OutStream& Annotation::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Annotation name=";
		name->prettyPrint(s, 0);

		if (args) args->prettyPrint(s << '\n', buf + 2, "args=");
		return s;
	}


	/*
	 * ast::GenericPart
	 */
	GenericPart::GenericPart(ptr<BasicBinding> n, ptr<Type> t) : name{ std::move(n) }, type{ std::move(t) } {}


	/*
	 * ast::TypeGeneric
	 */
	TypeGeneric::TypeGeneric(ptr<BasicBinding> b, ptr<Type> t, RelationType rel, VarianceType var)
		: GenericPart{ std::move(b), std::move(t) }, rel{ rel }, var{ var } {}
	OutStream& TypeGeneric::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.TypeGeneric (rel=" << rel._to_string() << ", var=" << var._to_string() << ')';
		name->prettyPrint(s << '\n', buf + 2, "binding=");
		if (type) type->prettyPrint(s << '\n', buf + 2, "type=");
		return s;
	}


	/*
	 * ast::ValueGeneric
	 */
	ValueGeneric::ValueGeneric(ptr<BasicBinding> b, ptr<Type> t, ptr<ValExpr> v)
		: GenericPart{ std::move(b), std::move(t) }, value{ std::move(v) } {}
	OutStream& ValueGeneric::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.ValueGeneric (type=" << (type != nullptr) << ", val=" << (value != nullptr) << ')';
		name->prettyPrint(s << '\n', buf + 2, "binding=");
		if (value) value->prettyPrint(s << '\n', buf + 2, "def=");
		if (type) type->prettyPrint(s << '\n', buf + 2, "type=");
		return s;
	}


	/*
	 * ast::Case
	 */
	Case::Case(ptr<PTuple> vs, ptr<ValExpr> e, ptr<ValExpr> if_g) : vars{ std::move(vs) }, expr{ std::move(e) }, if_guard{ std::move(if_g) } {}
	OutStream& Case::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Case";
		vars->prettyPrint(s << '\n', buf + 2, "pattern=");
		if (if_guard) if_guard->prettyPrint(s << '\n', buf + 2, "if=");
		return expr->prettyPrint(s << '\n', buf + 2, "expr=");
	}


	/*
	 * ast::ImporPiece
	 */
	OutStream& ImportPiece::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.ImportAny (_)";
	}


	/*
	 * ast::ImportName
	 */
	ImportName::ImportName(ptr<BasicBinding> n, ptr<BasicBinding> o) : name{ std::move(n) }, old_name{ std::move(o) } {}
	OutStream& ImportName::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.ImportName (";

		(old_name ? old_name : name)->prettyPrint(s, 0, "name=") << ')';
		if (old_name) name->prettyPrint(s << '\n', buf + 2, "rebind=");

		return s;
	}


	/*
	 * ast::ImportGroup
	 */
	ImportGroup::ImportGroup(std::deque<ptr<ImportPiece>> is) : imps{ std::move(is) } {}
	OutStream& ImportGroup::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.ImportGroup (size=" << imps.size() << ')';

		for (auto&& imp : imps)
			imp->prettyPrint(s << '\n', buf + 2);

		return s;
	}


	/*
	 * ast::Adt
	 */
	Adt::Adt(ptr<BasicBinding> n, ptr<TupleType> t) : name{ std::move(n) }, args{ std::move(t) } {}
	OutStream& Adt::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Adt (name=";
		name->prettyPrint(s, 0) << ")";

		if (args) args->prettyPrint(s << '\n', buf + 2, "args=");
		return s;
	}


	/*
	 * ast::Future
	 */
	Future::Future(bool f) : forwarded_from_fn{ f } {}
	OutStream& Future::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.FutureValue (fwd=" << forwarded_from_fn << ')';
	}

}