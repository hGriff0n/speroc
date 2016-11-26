#include "ast/control.h"
#include "ast/decor.h"
#include "ast/atoms.h"
#include "ast/names.h"

namespace spero::compiler::ast {

	/*
	 * ast::Branch
	 */
	Branch::Branch(ptr<ValExpr> test, ptr<ValExpr> body) : else_branch{nullptr} {
		addBranch(std::move(test), std::move(body));
	}
	void Branch::addBranch(ptr<ValExpr> test, ptr<ValExpr> body) {
		// TODO: assert else not filled
		if_branches.emplace_back(std::make_pair(std::move(test), std::move(body)));
	}
	void Branch::addBranch(ptr<ValExpr> body) {
		// TODO: assert else not filled
		else_branch = std::move(body);
	}
	OutStream& Branch::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Branch (";
		ValExpr::prettyPrint(s, 0);

		size_t cnt = 0;
		for (auto& branch : if_branches) {
			branch.first->prettyPrint(s << '\n', buf + 2, cnt++ ? "elsif=" : "if=");
			branch.second->prettyPrint(s << '\n', buf + 4, "body=");
		}

		if (else_branch) else_branch->prettyPrint(s << '\n', buf + 2, "else=");

		return s;
	}


	/*
	 * ast::Loop
	 */
	Loop::Loop(ptr<ValExpr> b) : body{ std::move(b) } {}
	OutStream& Loop::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Loop (";
		ValExpr::prettyPrint(s, 0);
		return body->prettyPrint(s << '\n', buf + 2, "body=");
	}


	/*
	 * ast::While
	 */
	While::While(ptr<ValExpr> test, ptr<ValExpr> body) : test{ std::move(test) }, body{ std::move(body) } {}
	OutStream& While::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.While (";
		ValExpr::prettyPrint(s, 0);
		test->prettyPrint(s << '\n', buf + 2, "cond=");
		return body->prettyPrint(s << '\n', buf + 2, "body=");
	}


	/*
	 * ast::For
	 */
	For::For(ptr<Pattern> p, ptr<ValExpr> g, ptr<ValExpr> b) : pattern{ std::move(p) }, generator{ std::move(g) }, body{ std::move(b) } {}
	OutStream& For::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.For (";
		ValExpr::prettyPrint(s, 0);

		pattern->prettyPrint(s << '\n', buf + 2, "vars=");
		generator->prettyPrint(s << '\n', buf + 2, "in=");
		return body->prettyPrint(s << '\n', buf + 2, "body=");
	}


	/*
	 * ast::Match
	 */
	Match::Match(ptr<ValExpr> s, std::deque<ptr<Case>> cs) : switch_expr{ std::move(s) }, cases{ std::move(cs) } {}
	OutStream& Match::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Match (";
		ValExpr::prettyPrint(s, 0);

		switch_expr->prettyPrint(s << '\n', buf + 2, "switch=");

		for (auto&& c : cases)
			c->prettyPrint(s << '\n', buf + 2, "case=");

		return s;
	}


	/*
	 * ast::Jump
	 */
	Jump::Jump(KeywordType k, ptr<ValExpr> e) : expr{ std::move(e) }, jmp{ k } {}


	/*
	 * ast::Wait
	 */
	Wait::Wait(ptr<ValExpr> e) : Jump{ KeywordType::WAIT, std::move(e) } {}
	OutStream& Wait::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Wait (";
		ValExpr::prettyPrint(s, 0);

		if (expr) expr->prettyPrint(s << '\n', buf + 2, "expr=");
		return s;
	}


	/*
	 * ast::Break
	 */
	Break::Break(ptr<ValExpr> e) : Jump{ KeywordType::BREAK, std::move(e) } {}
	OutStream& Break::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Break (";
		ValExpr::prettyPrint(s, 0);

		if (expr) expr->prettyPrint(s << '\n', buf + 2, "expr=");
		return s;
	}


	/*
	 * ast::Continue
	 */
	Continue::Continue(ptr<ValExpr> e) : Jump{ KeywordType::CONT, std::move(e) } {}
	OutStream& Continue::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Continue (";
		ValExpr::prettyPrint(s, 0);

		if (expr) expr->prettyPrint(s << '\n', buf + 2, "expr=");
		return s;
	}


	/*
	 * ast::Return
	 */
	Return::Return(ptr<ValExpr> e) : Jump{ KeywordType::RET, std::move(e) } {}
	OutStream& Return::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Return (";
		ValExpr::prettyPrint(s, 0);

		if (expr) expr->prettyPrint(s << '\n', buf + 2, "expr=");
		return s;
	}


	/*
	 * ast::YieldRet
	 */
	YieldRet::YieldRet(ptr<ValExpr> e) : Jump{ KeywordType::YIELD, std::move(e) } {}
	OutStream& YieldRet::prettyPrint(OutStream& s, size_t buf, std::string context) {
		s << std::string(buf, ' ') << context << "ast.Yield (";
		ValExpr::prettyPrint(s, 0);

		if (expr) expr->prettyPrint(s << '\n', buf + 2, "expr=");
		return s;
	}

}