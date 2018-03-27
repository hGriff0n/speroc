#pragma once

#include "parser/visitor.h"
#include "interface/CompilationState.h"
#include "util/asmjit.h"

namespace spero::compiler::gen {

	/*
	 * Visitor class to control basic emission of assembly code
	 *
	 * TODO: Split off some data into analysis phases
	 * TODO: Rewrite to utilize different IR structure
	 */
	// class AsmGenerator : public ast::Visitor<Assembler> {
	class AsmGenerator : public ast::Visitor {
		Assembler emit;
		compiler::CompilationState& state;

		analysis::SymTable globals;
		analysis::SymTable* current = &globals;

		void assign(std::string&, bool, Location);
		std::optional<ref_t<analysis::VarData>> scopedAccess(ast::Path&);

		public:
			// The constructor should except the data from the last pass (template parameter?)
			AsmGenerator(compiler::CompilationState&);
			Assembler get();

			// Base Nodes
			virtual void visit(ast::Ast&) final;

			// Literals
			virtual void visitBool(ast::Bool&) final;
			virtual void visitByte(ast::Byte&) final;
			virtual void visitFloat(ast::Float&) final;
			virtual void visitInt(ast::Int&) final;
			virtual void visitChar(ast::Char&) final;
			//virtual void visitString(ast::String&) final;

			// Atoms
			virtual void visitBlock(ast::Block&) final;

			// Names
			virtual void visitVariable(ast::Variable&) final;
			virtual void visitAssignName(ast::AssignName&) final;

			// Types

			// Decorations

			// Control

			// Statements
			virtual void visitVarAssign(ast::VarAssign&) final;

			// Expressions
			virtual void visitInAssign(ast::InAssign&) final;
			virtual void visitFunction(ast::Function&) final;
			virtual void visitBinOpCall(ast::BinOpCall&) final;
			virtual void visitUnOpCall(ast::UnOpCall&) final;
	};

}