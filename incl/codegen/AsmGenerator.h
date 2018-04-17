#pragma once

#include "parser/AstVisitor.h"
#include "interface/CompilationState.h"
#include "util/asmjit.h"

namespace spero::compiler::gen {

	/*
	 * AstVisitor class to control basic emission of assembly code
	 *
	 * TODO: Split off some data into analysis phases
	 * TODO: Rewrite to utilize different IR structure
	 */
	// This is an idea to formalize the conception that Visitors "produce" an analysis result
	// class AsmGenerator : public ast::AstVisitor<Assembler> {
	class AsmGenerator : public ast::AstVisitor {
		Assembler emit;
		compiler::CompilationState& state;

		std::unique_ptr<analysis::SymTable> globals;
		analysis::SymTable* current = nullptr;

		public:
			// The constructor should except the data from the last pass (template parameter?)
			AsmGenerator(compiler::CompilationState&);
			AsmGenerator(std::unique_ptr<analysis::SymTable> globals, compiler::CompilationState& state);
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