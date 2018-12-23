#pragma once

#pragma warning(push, 0)
//#include <llvm/ADT/APFloat.h>
//#include <llvm/ADT/STLExtras.h>
//#include <llvm/IR/BasicBlock.h>
//#include <llvm/IR/Constants.h>
//#include <llvm/IR/DerivedTypes.h>
//#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
//#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#pragma warning(pop)

#include "parser/AstVisitor.h"
#include "interface/CompilationState.h"
#include "util/parser.h"
#include "util/ranges.h"

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
		llvm::LLVMContext context;
		llvm::IRBuilder<> builder;
		llvm::Module* translationUnit = nullptr;

		CompilationState& state;

		analysis::SymArena& arena;
		static constexpr analysis::SymIndex globals = 0;
		analysis::SymIndex current = globals;

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
			LlvmIrGenerator(analysis::SymArena& arena, CompilationState& state)
				: state{ state }, arena{ arena }, translationUnit{ new llvm::Module("speroc", context) }, builder{ context }
			{}

			llvm::Module* finalize() {
				return translationUnit;
			}

			// Literals
			//virtual void visitBool(ast::Bool& b) final {
			//	codegen = llvm::ConstantInt::get(context, llvm::APInt(1, b.val));
			//}
			//virtual void visitByte(ast::Byte& b) final {
			//	codegen = llvm::ConstantInt::get(context, llvm::APInt(sizeof(b.val), b.val));
			//}
			//virtual void visitFloat(ast::Float& f) final {
			//	codegen = llvm::ConstantFP::get(context, llvm::APFloat(f.val));
			//}
			virtual void visitInt(ast::Int& i) final {
				codegen = llvm::ConstantInt::get(context, llvm::APInt(sizeof(i.val), i.val, true));
			}
			//virtual void visitChar(ast::Char& c) final {
			//	codegen = llvm::ConstantInt::get(context, llvm::APInt(sizeof(c.val), c.val));
			//}
			//virtual void visitString(ast::String&) final {}

			// Atoms
			virtual void visitTuple(ast::Tuple& t) final {
				/*LlvmIrGenerator::visitValExpr(t);

				for (auto&& elem : t.elems) {
					elem->accept(*this);
				}*/
			}
			virtual void visitBlock(ast::Block&) final {}
			virtual void visitFunction(ast::Function& f) final {
				// TODO: Assemble arg types
				std::vector<llvm::Type*> arg_types;
				llvm::FunctionType* ft = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), arg_types, false);

				auto fn = llvm::Function::Create(ft, llvm::Function::ExternalLinkage,
					f.name->get(), translationUnit);
				//translationUnit->getFunctionList().addNodeToList(fn);

				// TODO: Add in arguments (again)
				//unsigned Idx = 0;
				//for (auto &Arg : F->args())
					//Arg.setName(Args[Idx++]);

				// TODO: Insert error checking

				llvm::BasicBlock* bb = llvm::BasicBlock::Create(context, "entry", fn);
				builder.SetInsertPoint(bb);

				for (auto&& arg : f.args) {
					// namedValues[arg.name] = arg;
				}

				auto retval = visitNode(*f.body);
				if (!retval) {
					fn->eraseFromParent();
					codegen = nullptr;
					state.log(compiler::ID::err, "Error in function body codegen, null returned <at {}>", f.loc);
					return;
				}

				builder.CreateRet(retval);
				llvm::verifyFunction(*fn);
				codegen = fn;
			}

			// Names
			virtual void visitVariable(ast::Variable&) final {}
			virtual void visitAssignName(ast::AssignName& a) final {
				
			}

			// Types

			// Decorations

			// Control
			virtual void visitIfBranch(ast::IfBranch&) final {}
			virtual void visitIfElse(ast::IfElse&) final {}

			// Statements
			//virtual void visitVarAssign(ast::VarAssign&) final {}

			// Expressions
			virtual void visitInAssign(ast::InAssign&) final {}
			virtual void visitBinOpCall(ast::BinOpCall& b) final {
				if (b.op == "=") {

				} else {
					auto lhs = visitNode(*b.lhs);
					auto rhs = visitNode(*b.rhs);
					
					// NOTE: For the moment, I'm assuming that we'll ensure that everything types to llvm
					if (b.op == "+") {
						codegen = builder.CreateAdd(lhs, rhs);

					} else if (b.op == "-") {
						codegen = builder.CreateSub(lhs, rhs);
					}
				}
			}
			virtual void visitUnOpCall(ast::UnOpCall&) final {}
			virtual void visitFnCall(ast::FnCall& f) final {
				visitValExpr(f);

				std::vector<llvm::Value*> args;
				for (auto&& elem : f.arguments->elems) {
					args.push_back(visitNode(*elem));
					if (!args.back()) {
						state.log(compiler::ID::err, "Error in codegen for fn call arguments, null node produced <at {}>", f.loc);
						return;
					}
				}

				if (!util::is_type<ast::Variable>(f.callee)) {
					state.log(compiler::ID::err, "Calling non-var-bound functions is currently not supported <at {}>", f.loc);
					return;
				}

				auto* fn = util::view_as<ast::Variable>(f.callee);

				// TODO: Figure out how functions are registered
				// TODO: How will this work with modules? (`Function::ExternalLinkage`)
				llvm::Function* callee = translationUnit->getFunction(fn->name->elems.back()->name.get());
				if (!callee) {
					state.log(compiler::ID::err, "No prototype found for function <at {}>", f.loc);
					return;
				}

				codegen = builder.CreateCall(callee, args);
			}
	};

}