#pragma once

#pragma warning(push, 0)
//#include <llvm/ADT/APFloat.h>
//#include <llvm/ADT/STLExtras.h>
//#include <llvm/IR/BasicBlock.h>
//#include <llvm/IR/Constants.h>
//#include <llvm/IR/DerivedTypes.h>
//#include <llvm/IR/Function.h>
//#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
//#include <llvm/IR/Type.h>
//#include <llvm/IR/Verifier.h>
#pragma warning(pop)

#include "parser/AstVisitor.h"
#include "interface/CompilationState.h"

namespace spero::compiler::gen {

	/*
	 * AstVisitor class to control translation into llvm ir
	 */
	class LlvmIrGenerator : public ast::AstVisitor {
		llvm::LLVMContext context;
		llvm::Module* translationUnit = nullptr;
		CompilationState& state;

		analysis::SymArena& arena;
		static constexpr analysis::SymIndex globals = 0;
		analysis::SymIndex current = globals;

		public:
			LlvmIrGenerator(analysis::SymArena& arena, CompilationState& state)
				: state{ state }, arena{ arena }, translationUnit{ new llvm::Module("speroc", context) }
			{}

			llvm::Module* finalize() {
				return translationUnit;
			}

			// Literals
			virtual void visitBool(ast::Bool& b) final {}
			virtual void visitByte(ast::Byte& b) final {}
			virtual void visitFloat(ast::Float& f) final {}
			virtual void visitInt(ast::Int& i) final {}
			virtual void visitChar(ast::Char& c) final {}
			//virtual void visitString(ast::String&) final {}

			// Atoms
			virtual void visitTuple(ast::Tuple&) final {}
			virtual void visitBlock(ast::Block&) final {}
			virtual void visitFunction(ast::Function&) final {}

			// Names
			virtual void visitVariable(ast::Variable&) final {}
			virtual void visitAssignName(ast::AssignName&) final {}

			// Types

			// Decorations

			// Control
			virtual void visitIfBranch(ast::IfBranch&) final {}
			virtual void visitIfElse(ast::IfElse&) final {}

			// Statements
			//virtual void visitVarAssign(ast::VarAssign&) final {}

			// Expressions
			virtual void visitInAssign(ast::InAssign&) final {}
			virtual void visitBinOpCall(ast::BinOpCall&) final {}
			virtual void visitUnOpCall(ast::UnOpCall&) final {}
			virtual void visitFnCall(ast::FnCall&) final {}
	};

}