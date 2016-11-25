//#include "ast/atoms.h"
#include "ast/all.h"

namespace spero::compiler::ast {

	/*
	 * ast::Tuple
	 */
	Tuple::Tuple(std::deque<ptr<ValExpr>> vals) : Sequence{ std::move(vals) } {}
	OutStream& Tuple::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast..Tuple";
	}

	
	/*
	 * ast::Array
	 */
	Array::Array(std::deque<ptr<ValExpr>> vals) : Sequence{ std::move(vals) } {}
	OutStream& Array::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.Array";
	}


	/*
	 * ast::Block
	 */
	Block::Block(std::deque<ptr<Ast>> es) : Sequence{ std::move(es) } {}
	OutStream& Block::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Block (size=" << exprs.size() << ") {";
		for (auto&& e : exprs)
			e->prettyPrint(s << "\n", buf + 2);
		return s << "\n" << std::string(buf, ' ') << "}";
	}


	/*
	 * ast::TypeExtension
	 */
	TypeExtension::TypeExtension(ptr<Block> b) : cons{}, body{ std::move(b) } {}
	TypeExtension::TypeExtension(ptr<Tuple> c, ptr<Block> b) : cons{ std::move(c) }, body{ std::move(b) } {}
	OutStream& TypeExtension::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.TypeExtension";
	}


	/*
	 * ast::FnCall
	 */
	FnCall::FnCall(ptr<Ast> c, ptr<TypeExtension> t, ptr<Tuple> a, ptr<Array> i)
		: caller{ std::move(c) }, anon{ std::move(t) }, args{ std::move(a) }, inst{ std::move(i) } {}
	OutStream& FnCall::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.FnCall";
	}


	/*
	 * ast::Range
	 */
	Range::Range(ptr<ValExpr> start, ptr<ValExpr> stop) : start{ std::move(start) }, stop{ std::move(stop) }, step{} {}
	OutStream& Range::prettyPrint(OutStream& s, size_t buf, std::string context) {
		return s << std::string(buf, ' ') << context << "ast.Range";
	}

}