//#include "ast/control.h"
//#include "ast/names.h"
//#include "ast/decor.h"
#include "ast/all.h"

namespace spero::compiler::ast {

	/*
	 * ast::Branch
	 */
	Branch::Branch(ptr<ValExpr> test, ptr<ValExpr> body) : else_branch{} {
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
	OutStream Branch::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::Loop
	 */
	Loop::Loop(ptr<ValExpr> b) : body{ std::move(b) } {}
	OutStream Loop::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::While
	 */
	While::While(ptr<ValExpr> test, ptr<ValExpr> body) : test{ std::move(test) }, body{ std::move(body) } {}
	OutStream While::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::For
	 */
	For::For(ptr<Pattern> p, ptr<ValExpr> g, ptr<ValExpr> b) : pattern{ std::move(p) }, generator{ std::move(g) }, body{ std::move(b) } {}
	OutStream For::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::Match
	 */
	Match::Match(ptr<ValExpr> s, std::deque<ptr<Case>> cs) : switch_expr{ std::move(s) }, cases{ std::move(cs) } {}
	OutStream Match::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::Jump
	 */
	Jump::Jump(KeywordType k) : expr{}, jmp{ k } {}
	Jump::Jump(KeywordType k, ptr<ValExpr> e) : expr{ std::move(e) }, jmp{ k } {}


	/*
	 * ast::Wait
	 */
	Wait::Wait(ptr<ValExpr> e) : Jump{ KeywordType::WAIT, std::move(e) } {}
	OutStream Wait::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::Break
	 */
	Break::Break(ptr<ValExpr> e) : Jump{ KeywordType::BREAK, std::move(e) } {}
	OutStream Break::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::Continue
	 */
	Continue::Continue(ptr<ValExpr> e) : Jump{ KeywordType::CONT, std::move(e) } {}
	OutStream Continue::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::Return
	 */
	Return::Return(ptr<ValExpr> e) : Jump{ KeywordType::RET, std::move(e) } {}
	OutStream Return::prettyPrint(OutStream s, size_t buf) {
		return s;
	}


	/*
	 * ast::YieldRet
	 */
	YieldRet::YieldRet(ptr<ValExpr> e) : Jump{ KeywordType::YIELD, std::move(e) } {}
	OutStream YieldRet::prettyPrint(OutStream s, size_t buf) {
		return s;
	}

}