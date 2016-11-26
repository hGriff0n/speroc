#include "ast/atoms.h"
#include "ast/decor.h"
#include "ast/names.h"

namespace spero::compiler::ast {

	/*
	 * ast::Tuple
	 */
	Tuple::Tuple(std::deque<ptr<ValExpr>> vals) : Sequence{ std::move(vals) } {}
	OutStream& Tuple::prettyPrint(OutStream& s, size_t buf, std::string context) {
		// TODO: Need to differentiate Tuple, Array, and Block in prettyPrinting
		s << std::string(buf, ' ') << context << "ast.Tuple" << exprs.size() << " (";
		for (auto&& e : exprs)
			e->prettyPrint(s << '\n', buf + 2) << ",";

		if (exprs.size()) s << '\n' << std::string(buf, ' ');
		return s << ')';
	}

	
	/*
	 * ast::Array
	 */
	Array::Array(std::deque<ptr<ValExpr>> vals) : Sequence{ std::move(vals) } {}
	OutStream& Array::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Array" << exprs.size() << " [type={}] [";
		for (auto&& e : exprs)
			e->prettyPrint(s << '\n', buf + 2) << ",";

		if (exprs.size()) s << '\n' << std::string(buf, ' ');
		return s << ']';
	}


	/*
	 * ast::Block
	 */
	Block::Block(std::deque<ptr<Ast>> es) : Sequence{ std::move(es) } {}
	OutStream& Block::prettyPrint(OutStream& s, size_t buf, std::string context) {
		// TODO: value(Block) == value(Block.last), I want to add some special output
		s << std::string(buf, ' ') << context << "ast.Block (size=" << exprs.size() << ") {";
		for (auto&& e : exprs)
			e->prettyPrint(s << '\n', buf + 2);

		if (exprs.size()) s << '\n' << std::string(buf, ' ');
		return s << '}';
	}


	/*
	 * ast::TypeExtension
	 */
	TypeExtension::TypeExtension(ptr<Block> b) : cons{}, body{ std::move(b) } {}
	TypeExtension::TypeExtension(ptr<Tuple> c, ptr<Block> b) : cons{ std::move(c) }, body{ std::move(b) } {}
	OutStream& TypeExtension::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.TypeExtension";
		if (cons) cons->prettyPrint(s << '\n', buf + 2, "cons=");
		return body->prettyPrint(s << '\n', buf + 2, "type=");
	}


	/*
	 * ast::FnCall
	 */
	FnCall::FnCall(ptr<Ast> c, ptr<TypeExtension> t, ptr<Tuple> a, ptr<Array> i)
		: caller{ std::move(c) }, anon{ std::move(t) }, args{ std::move(a) }, inst{ std::move(i) } {}
	OutStream& FnCall::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.FnCall (anon="
			<< (bool)anon << ", args=" << (bool)args << ", inst=" << (bool)inst << ", ";
		ValExpr::prettyPrint(s, buf) << '\n';
		caller->prettyPrint(s, buf + 2, "caller=");

		if (anon) anon->prettyPrint(s << '\n', buf + 2, "anon=");
		if (args) args->prettyPrint(s << '\n', buf + 2, "args=");
		if (inst) inst->prettyPrint(s << '\n', buf + 2, "inst=");
		return s;
	}


	/*
	 * ast::Range
	 */
	Range::Range(ptr<ValExpr> start, ptr<ValExpr> stop) : start{ std::move(start) }, stop{ std::move(stop) }, step{ nullptr } {}
	OutStream& Range::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Range (";
		ValExpr::prettyPrint(s, buf);

		start->prettyPrint(s << '\n', buf + 2, "start=");
		if (step) step->prettyPrint(s << '\n', buf + 2, "step=");
		stop->prettyPrint(s << '\n', buf + 2, "stop=");
		return s;
	}


	/*
	 * ast::UnApp
	 */
	UnaryApp::UnaryApp(ptr<ValExpr> e, UnaryType t) : op{ t }, expr{ std::move(e) } {}
	OutStream& UnaryApp::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.UnaryApp (op=" << op._to_string();
		ValExpr::prettyPrint(s << ", ", buf);

		return expr->prettyPrint(s << '\n', buf + 2, "expr=");
	}

}