//#include "ast/decor.h"
//#include "ast/atoms.h"
//#include "ast/names.h"
//#include "ast/types.h"
#include "ast/all.h"

namespace spero::compiler::ast {

	/*
	 * ast::Annotation
	 */
	Annotation::Annotation(ptr<BasicBinding> n, ptr<Tuple> t, bool g) : name{ std::move(n) }, args{ std::move(t) }, global{ g } {}
	OutStream& Annotation::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.Annotation";
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
		return s << std::string(buf, ' ') << context << "ast.TypeGeneric";
	}


	/*
	 * ast::ValueGeneric
	 */
	ValueGeneric::ValueGeneric(ptr<BasicBinding> b, ptr<Type> t, ptr<ValExpr> v)
		: GenericPart{ std::move(b), std::move(t) }, value{ std::move(v) } {}
	OutStream& ValueGeneric::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.ValueGeneric";
	}


	/*
	 * ast::Case
	 */
	Case::Case(ptr<ValExpr> e) : expr{ std::move(e) } {}
	Case::Case(std::deque<ptr<Pattern>> vs, ptr<ValExpr> e) : vars{ std::move(vs) }, expr{ std::move(e) } {}
	OutStream& Case::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.Case";
	}


	/*
	 * ast::ImporPiece
	 */
	OutStream& ImportPiece::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.ImportPiece";
	}


	/*
	 * ast::ImportName
	 */
	ImportName::ImportName(ptr<BasicBinding> n) : name{ std::move(n) }, old_name{} {}
	ImportName::ImportName(ptr<BasicBinding> n, ptr<BasicBinding> o) : name{ std::move(n) }, old_name{ std::move(o) } {}
	OutStream& ImportName::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.ImportName";
	}


	/*
	 * ast::ImportGroup
	 */
	ImportGroup::ImportGroup(std::deque<ptr<ImportPiece>> is) : imps{ std::move(is) } {}
	OutStream& ImportGroup::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.ImportGroup";
	}


	/*
	 * ast::Adt
	 */
	Adt::Adt(ptr<BasicBinding> n, ptr<TupleType> t) : name{ std::move(n) }, args{ std::move(t) } {}
	OutStream& Adt::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.Adt";
	}

}