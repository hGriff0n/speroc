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
	 * TODO: How do the `IRBuilder`, `Module*`, and `LLVMContext` classes interoperate?
	 * TODO: Need to investigate whether I should transform the results into a separate "IR" first?
	 *   This would basically be a vector of `AST*`, so we could declare functions first (maybe split out all declarations instead?)
	 */
	class LlvmIrGenerator : public ast::AstVisitor {
		llvm::IRBuilder<> builder;
		llvm::Module* translationUnit = nullptr;
		//std::unique_ptr<llvm::Module*>

		CompilationState& state;

		//analysis::SymArena& arena;
		//static constexpr analysis::SymIndex globals = 0;
		//analysis::SymIndex current = globals;

		protected:
			llvm::Value* codegen = nullptr;
			llvm::Value* visitNode(ast::Ast& a) {
				llvm::Value* last = codegen;
				codegen = nullptr;

				a.accept(*this);

				llvm::Value* ret = codegen;
				codegen = last;
				return ret;
			}

		public:
			/*LlvmIrGenerator(analysis::SymArena& arena, CompilationState& state)
				: state{ state }, arena{ arena }, translationUnit{ new llvm::Module("speroc", context) }, builder{ context }
			{}*/
			LlvmIrGenerator(CompilationState& state)
				: state{ state }, translationUnit{ new llvm::Module("speroc", state.getContext()) }, builder{ state.getContext() }
			{}

			llvm::Module* finalize() {
				return translationUnit;
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
			virtual void visitInt(ast::Int& i) final {
				codegen = llvm::ConstantInt::get(state.getContext(), llvm::APInt(sizeof(i.val) * 8, i.val, true));
			}

			// Expressions
			virtual void visitBinOpCall(ast::BinOpCall& b) final {
				if (b.op == "=") {
					// TODO: Not sure how to handle this in llvm ir (due to ssa)
					// We sort of mimic ssa assignments in the current var passes
					// But it doesn't get reduced to one "name" (probably should do that)
					return;
				}

				auto lhs = visitNode(*b.lhs);
				auto rhs = visitNode(*b.rhs);

				if (b.op == "+") {
					codegen = builder.CreateAdd(lhs, rhs);

				} else if (b.op == "-") {
					codegen = builder.CreateSub(lhs, rhs);
				}
			}
	};

}