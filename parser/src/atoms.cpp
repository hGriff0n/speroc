//#include "ast/atoms.h"
#include "ast/all.h"

namespace spero::compiler::ast {

	/*
	 * ast::Tuple
	 */
	Tuple::Tuple(std::deque<ptr<ValExpr>> vals) : Sequence{ std::move(vals) } {}
	OutStream Tuple::prettyPrint(OutStream s, size_t buf) {
		return s;
	}

	
	/*
	 * ast::Array
	 */
	Array::Array(std::deque<ptr<ValExpr>> vals) : Sequence{ std::move(vals) } {}
	OutStream Array::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::Block
	 */
	Block::Block(std::deque<ptr<Ast>> es) : Sequence{ std::move(es) } {}
	OutStream Block::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::TypeExtension
	 */
	TypeExtension::TypeExtension(ptr<Block> b) : cons{}, body{ std::move(b) } {}
	TypeExtension::TypeExtension(ptr<Tuple> c, ptr<Block> b) : cons{ std::move(c) }, body{ std::move(b) } {}
	OutStream TypeExtension::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::FnCall
	 */
	FnCall::FnCall(ptr<Ast> c, ptr<TypeExtension> t, ptr<Tuple> a, ptr<Array> i)
		: caller{ std::move(c) }, anon{ std::move(t) }, args{ std::move(a) }, inst{ std::move(i) } {}
	OutStream FnCall::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::Range
	 */
	Range::Range(ptr<ValExpr> start, ptr<ValExpr> stop) : start{ std::move(start) }, stop{ std::move(stop) }, step{} {}
	OutStream Range::prettyPrint(OutStream s, size_t buf) {
		return s;
	}

}