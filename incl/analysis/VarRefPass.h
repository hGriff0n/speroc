#pragma once

#include "parser/AstVisitor.h"
#include "interface/CompilationState.h"
#include "util/analysis.h"

namespace spero::compiler::analysis {

	/*
	 * This pass handles variable resolution and other such operations
	 * NOTE: Defined as a separate pass from VarDeclPass in order to accomodate "use-before-dec" cases
	 */
	class VarRefPass : public ast::AstVisitor {
		compiler::CompilationState& state;

		std::unique_ptr<SymTable> globals;
		SymTable* current = nullptr;

		// Track the current scoping context to be able to tailor analysis
		ScopingContext context = ScopingContext::GLOBAL;

		public:
			VarRefPass(compiler::CompilationState& state, std::unique_ptr<SymTable> table);
			std::unique_ptr<SymTable> finalize();

			// Atoms
			virtual void visitBlock(ast::Block&) final;

			// Expressions
			virtual void visitVariable(ast::Variable&) final;
			virtual void visitBinOpCall(ast::BinOpCall&) final;

			// Statements
			virtual void visitInAssign(ast::InAssign&) final;
	};

}
