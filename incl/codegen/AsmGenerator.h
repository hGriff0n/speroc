#pragma once

#include "parser/visitor.h"
#include "AsmEmitter.h"
#include "interface/CompilationState.h"

namespace spero::compiler::gen {

	/*
	 * Visitor class to control basic emission of assembly code
	 *
	 * TODO: Split off some data into analysis phases
	 * TODO: Rewrite to utilize different IR structure
	 */
	class AsmGenerator : public ast::Visitor {
		AsmEmitter emit;
		std::ostream& out;

		compiler::CompilationState& state;

		analysis::SymTable globals;
		analysis::SymTable* current = &globals;

		void assign(std::string&, bool, ast::Location);

		public:
			AsmGenerator(std::ostream&, compiler::CompilationState&);

			// Base Nodes
			virtual void visit(ast::Ast&) final;

			// Literals
			virtual void visitBool(ast::Bool&) final;
			virtual void visitByte(ast::Byte&) final;
			virtual void visitFloat(ast::Float&) final;
			virtual void visitInt(ast::Int&) final;
			virtual void visitChar(ast::Char&) final;
			//virtual void visitString(ast::String&) final;

			// Names
			virtual void visitVariable(ast::Variable&) final;
			virtual void visitAssignName(ast::AssignName&) final;

			// Types

			// Decorations

			// Control
			virtual void visitBlock(ast::Block&) final;

			// Statements
			virtual void visitFunction(ast::Function&) final;
			virtual void visitBinOpCall(ast::BinOpCall&) final;
			virtual void visitUnOpCall(ast::UnOpCall&) final;
			virtual void visitVarAssign(ast::VarAssign&) final;
	};

}