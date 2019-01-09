#pragma once

#include "parser/AstVisitor.h"
#include "interface/CompilationState.h"
#include "analysis/AnalysisState.h"

namespace spero::analysis {

	/*
	 * This pass handles variable resolution and other such operations
	 * NOTE: Defined as a separate pass from VarDeclPass in order to accomodate "use-before-dec" cases
	 */
	class VarRefPass : public compiler::ast::AstVisitor {
		compiler::CompilationState& state;
		analysis::AnalysisState& dictionary;

		SymIndex current = GLOBAL_SYM_INDEX;

		// Track the current scoping context to be able to tailor analysis
		ScopingContext context = ScopingContext::GLOBAL;
		
		public:
			VarRefPass(compiler::CompilationState& state, AnalysisState& dict);

			// Atoms
			virtual void visitBlock(compiler::ast::Block&) final;
			virtual void visitFunction(compiler::ast::Function&) final;

			// Expressions
			virtual void visitVariable(compiler::ast::Variable&) final;
			virtual void visitBinOpCall(compiler::ast::BinOpCall&) final;

			// Statements
			virtual void visitInAssign(compiler::ast::InAssign&) final;
			virtual void visitTypeAssign(compiler::ast::TypeAssign&) final;
	};

}
