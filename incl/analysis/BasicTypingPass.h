#pragma once

#include "parser/AstVisitor.h"
#include "interface/CompilationState.h"
#include "util/analysis.h"
#include "analysis/AnalysisState.h"

namespace spero::analysis {

	/*
	 * Basic typing ast pass that fills in visible type information
	 *   and performs some simplistic type checking
	 *
	 * NOTE: This is just a very basic pass to get an understanding of how analysis types work
	 *   This will definitely be changed in the future, once I have functions and modules working
	 *   The biggest issue is in defining how passes should be allocated / ordered
	 */
	class BasicTypingPass : public compiler::ast::AstVisitor {
		compiler::CompilationState& state;
		analysis::AnalysisState& dictionary;

		SymTable* current = nullptr;

		public:
			BasicTypingPass(compiler::CompilationState& state, AnalysisState& dict);

			// Atoms
			virtual void visitInt(compiler::ast::Int&) final;
			virtual void visitBool(compiler::ast::Bool&) final;
			virtual void visitString(compiler::ast::String&) final;

			// Decorations
			virtual void visitTypeAnnotation(compiler::ast::TypeAnnotation&) final;

			// Statements
			virtual void visitBinOpCall(compiler::ast::BinOpCall&) final;
	};

}