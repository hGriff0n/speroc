#pragma once

// NOTE: Always surround llvm includes with these commands to disable warning reporting
// As they are both very numerous, not our concern, and actually prevent compilation (due to -werror)
#pragma warning(push, 0)
#pragma warning(disable:4996)
//#pragma warning(pop)
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#pragma warning(pop)

#include "parser/AstVisitor.h"
#include "interface/CompilationState.h"
#include "util/parser.h"
#include "util/ranges.h"

#include <iostream>

namespace spero::compiler::gen {

	/*
	 * AstVisitor class to control translation into llvm ir
	 *
	 * TODO: Need to come up with a better "translation" for this (the visitor pattern doesn't seem helpful)
	 * TODO: Need to investigate whether I should transform the results into a separate "IR" first?
	 *   This would basically be a vector of `AST*`, so we could declare functions first (maybe split out all declarations instead?)
	 */
	class LlvmIrGenerator : public ast::AstVisitor {
		llvm::IRBuilder<> builder;
		std::unique_ptr<llvm::Module> translationUnit;

		CompilationState& state;

		analysis::SymArena& arena;
		static constexpr analysis::SymIndex globals = 0;
		analysis::SymIndex current = globals;

		protected:
			llvm::Value* codegen = nullptr;
			llvm::Value* visitNode(ast::Ast&);

		public:
			LlvmIrGenerator(analysis::SymArena& arena, CompilationState& state);

			std::unique_ptr<llvm::Module> finalize() {
				return std::move(translationUnit);
			}

			llvm::Module* getTempModulePtr() {
				return translationUnit.get();
			}

			llvm::IRBuilder<>& getBuilder() {
				return builder;
			}

			llvm::LLVMContext& getContext() {
				return state.getContext();
			}

			void createDefaultRetInst() {
				builder.CreateRet(codegen);
				codegen = nullptr;
			}

			// Literals
			virtual void visitInt(ast::Int& i) final;

			// Atoms
			virtual void visitTuple(ast::Tuple&) final;
			virtual void visitBlock(ast::Block&) final;
			virtual void visitFunction(ast::Function&) final;

			// Names
			virtual void visitVariable(ast::Variable&) final;
			virtual void visitAssignName(ast::AssignName&) final;

			// Decorations
			virtual void visitArgument(ast::Argument&) final;

			// Control
			virtual void visitIfBranch(ast::IfBranch&) final;
			virtual void visitIfElse(ast::IfElse&) final;
			
			// Statements
			virtual void visitVarAssign(ast::VarAssign& v) final;

			// Expressions
			virtual void visitInAssign(ast::InAssign&) final;
			virtual void visitBinOpCall(ast::BinOpCall&) final;
			virtual void visitUnOpCall(ast::UnOpCall&) final;
			virtual void visitFnCall(ast::FnCall&) final;
	};

}