#pragma once

#include "parser/AstVisitor.h"
#include "interface/CompilationState.h"
#include "util/asmjit.h"

namespace spero::compiler::gen {

	/*
	 * AstVisitor class to control basic emission of assembly code
	 *
	 * TODO: Rewrite to utilize different IR structure
	 *   The final assembly production will be based off of an MIR representation, not the AST
	 */
	// This is an idea to formalize the conception that Visitors "produce" an analysis result
	// class AsmGenerator : public ast::AstVisitor<Assembler> {
	class AsmGenerator : public ast::AstVisitor {
		Assembler emit;
		CompilationState& state;

		std::unique_ptr<analysis::SymTable> globals;
		analysis::SymTable* current = nullptr;

		public:
			// The constructor should expect the data from the last pass (template parameter?)
			AsmGenerator(std::unique_ptr<analysis::SymTable> globals, CompilationState& state);
			Assembler finalize();

			// Literals
			virtual void visitBool(ast::Bool&) final;
			virtual void visitByte(ast::Byte&) final;
			virtual void visitFloat(ast::Float&) final;
			virtual void visitInt(ast::Int&) final;
			virtual void visitChar(ast::Char&) final;
			//virtual void visitString(ast::String&) final;

			// Atoms
			virtual void visitBlock(ast::Block&) final;
			virtual void visitFunction(ast::Function&) final;

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
			virtual void visitBinOpCall(ast::BinOpCall&) final;
			virtual void visitUnOpCall(ast::UnOpCall&) final;
	};

}