#pragma once

#include "parser/AstVisitor.h"
#include "interface/CompilationState.h"
#include "util/analysis.h"

namespace spero::analysis {

	/*
	 * This pass handles variable resolution and other such operations
	 * NOTE: Defined as a separate pass from VarDeclPass in order to accomodate "use-before-dec" cases
	 */
	class VarRefPass : public compiler::ast::AstVisitor {
		compiler::CompilationState& state;

		std::unique_ptr<SymTable> globals;
		SymTable* current = nullptr;

		// Track the current scoping context to be able to tailor analysis
		ScopingContext context = ScopingContext::GLOBAL;

		public:
			VarRefPass(compiler::CompilationState& state, std::unique_ptr<SymTable> table);
			std::unique_ptr<SymTable> finalize();

			// Atoms
			virtual void visitBlock(compiler::ast::Block&) final;

			// Expressions
			virtual void visitVariable(compiler::ast::Variable&) final;
			virtual void visitBinOpCall(compiler::ast::BinOpCall&) final;

			// Statements
			virtual void visitInAssign(compiler::ast::InAssign&) final;
	};

}
