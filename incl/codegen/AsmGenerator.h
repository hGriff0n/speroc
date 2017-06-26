#pragma once

#include "parser/visitor.h"
#include "AsmEmitter.h"
#include "SymTable.h"
#include "../util/CompilationState.h"

namespace spero::compiler::gen {

	/*
	 * Visitor class to control basic emission of assembly code
	 *
	 * TODO: Split off some data into analysis phases
	 * TODO: Rewrite to utilize different IR structure
	 */
	class AsmGenerator : public ast::Visitor {
		AsmEmitter emit;
		CompilationState& state;

		analysis::SymTable globals;
		analysis::SymTable* current = &globals;

		public:
			AsmGenerator(std::ostream&, CompilationState&);

			// Base Nodes
			virtual void accept(ast::Ast&) final;

			// Literals
			virtual void acceptBool(ast::Bool&) final;
			virtual void acceptByte(ast::Byte&) final;
			virtual void acceptFloat(ast::Float&) final;
			virtual void acceptInt(ast::Int&) final;
			virtual void acceptChar(ast::Char&) final;
			//virtual void acceptString(ast::String&) final;

			// Names
			virtual void acceptVariable(ast::Variable&) final;
			virtual void acceptAssignName(ast::AssignName&) final;
			virtual void acceptAssignTuple(ast::AssignTuple&) final;

			// Types

			// Decorations

			// Control
			virtual void acceptBlock(ast::Block&) final;

			// Statements
			virtual void acceptFnBody(ast::FnBody&) final;
			virtual void acceptBinOpCall(ast::BinOpCall&) final;
			virtual void acceptUnaryOpApp(ast::UnaryOpApp&) final;
			virtual void acceptVarAssign(ast::VarAssign&) final;
	};

}