#include "codegen/LlvmIrGenerator.h"

#include "util/parser.h"
#include "util/ranges.h"

namespace spero::compiler::gen {
	using namespace llvm;

	LlvmIrGenerator::LlvmIrGenerator(analysis::SymArena& arena, CompilationState& state)
		: state{ state }, arena{ arena }, translationUnit{ std::make_unique<llvm::Module>("speroc", state.getContext()) }, builder{ state.getContext() }
	{}

	Value* LlvmIrGenerator::visitNode(ast::Ast& a) {
		auto last = codegen;
		codegen = nullptr;

		a.accept(*this);

		auto ret = codegen;
		codegen = last;
		return ret;
	}

	//
	// Literals
	//
	void LlvmIrGenerator::visitInt(ast::Int& i) {
		codegen = ConstantInt::get(state.getContext(), APInt(sizeof(i.val) * 8, i.val, true));
	}

	//
	// Atoms
	//
	void LlvmIrGenerator::visitTuple(ast::Tuple& t) {
		// TODO: Needs more research on how to handle tuples
		// May want to split this out into multiple expressions (depending on analysis work)
		// NOTE: I can extend the llvm type system with tuples (can I?)
		/*for (auto& elem : t.elems) {
			auto n = visitNode(*elem);
			builder.CreateStore(n);
		}*/
	}
	void LlvmIrGenerator::visitBlock(ast::Block& b) {
		auto parent_scope = current;
		current = *b.locals;

		AstVisitor::visitBlock(b);

		current = parent_scope;
	}
	void LlvmIrGenerator::visitFunction(ast::Function& f) {
		// TODO: How will we handle cleanup code (maybe an ast transform would handle this?)

		// Retreive the existing insertion block as the function is automatically defined at the module level
		// This allows for defining functions inside of other functions within llvm ir
		// TODO: Is this what I actually want to have in spero?
		auto old_insert_point = builder.GetInsertBlock();

		// Handle the case where the function was already "declared" earlier
		auto fn = translationUnit->getFunction(f.name->get());
		if (!fn) {
			// TODO: Introduce a translation from spero::Type to llvm::Type (requires spero::Type)
			std::vector<Type*> arg_types{ f.args.size(), Type::getInt32Ty(state.getContext()) };
			auto fn_type = FunctionType::get(Type::getInt32Ty(state.getContext()), arg_types, false);

			fn = Function::Create(fn_type, Function::ExternalLinkage, f.name->get(), translationUnit.get());

			// TODO: This introduces bugs if the defining code, uses different names
			// However, I feel we should already handle this during the initial language checks
			auto idx = 0u;
			for (auto& arg : fn->args()) {
				arg.setName(f.args[idx]->name->name.get());
			}
		}

		if (!fn->empty()) {
			state.log(ID::err, "Function {} already has llvm definition <at {}>", f.name->get(), f.loc);
			return;
		}

		// Create the basic blocks
		auto entry_block = BasicBlock::Create(state.getContext(), "entry", fn);
		builder.SetInsertPoint(entry_block);

		// TODO: Map function argument to symbol table
		if (auto retval = visitNode(*f.body)) {
			// TODO: Codegen cleanup code
			builder.CreateRet(retval);

			verifyFunction(*fn);
			codegen = fn;

		} else {
			fn->eraseFromParent();
			codegen = nullptr;
		}

		// Restore the old insertion point
		builder.SetInsertPoint(old_insert_point);
	}

	//
	// Names
	//
	void LlvmIrGenerator::visitVariable(ast::Variable& v) {
		// TODO: Reduce the variable access at this stage to a flat map lookup

		auto[def_table, iter] = analysis::lookup(arena, current, *v.name);
		auto& path_part = **iter;
		auto nvar = arena[def_table].get(path_part.name, nullptr, path_part.loc, path_part.ssa_index);

		if (auto symbol = std::get_if<ref_t<analysis::SymbolInfo>>(&*nvar)) {
			if (symbol && symbol->get().storage) {
				codegen = builder.CreateLoad(symbol->get().storage);
			}
		}
	}
	void LlvmIrGenerator::visitAssignName(ast::AssignName& a) {
		if (!isa<Function>(codegen)) {
			// TODO: By this stage we should probably reduce the scope tables to a flatter map
			// And not require the usage of "generic" type value information
			// This would also "validate" the assumptions we make about the symbol existing
			opt_t<size_t> ssa_index = std::nullopt;
			auto nvar = arena[current].get(a.var->name, nullptr, a.var->loc, ssa_index);

			// We can assume that the symbol exists if we get to this point (VarDeclPass would automatically insert it)
			auto& symbol = std::get<ref_t<analysis::SymbolInfo>>(*nvar).get();
			switch (arena[current].context()) {
				case analysis::ScopingContext::GLOBAL: {
					auto initializer = dyn_cast<Constant>(codegen);
					auto storage = new GlobalVariable(
						*translationUnit, Type::getInt32Ty(state.getContext()), a.is_mut,
						GlobalVariable::ExternalLinkage, initializer, a.var->name.get()
					);
					// TODO: What do I do if the "initializer" isn't considered to be constant?
					symbol.storage = storage;
					//break;
				}
				case analysis::ScopingContext::SCOPE: {
					auto storage = builder.CreateAlloca(Type::getInt32Ty(state.getContext()), nullptr, a.var->name.get());
					codegen = builder.CreateStore(codegen, symbol.storage = storage);
					break;
				}
				case analysis::ScopingContext::TYPE:
				default:
					break;
			}

		} else {
			// TODO: Can we push the name assignment of the function to here?? (would simplify a lot)
			// However, we don't get the "already defined" check...
			//dyn_cast<Function>(codegen)->setName(a.var->name.get());
		}
	}

	//
	// Control
	//
	void LlvmIrGenerator::visitIfBranch(ast::IfBranch& b) {
		// TODO: This holds the structures for a conditional block
		// I don't think this is enough to generate code for llvm
		// NOTE: We never get a raw `IfBranch` from the parser (it's always in an `IfElse`)
	}
	void LlvmIrGenerator::visitIfElse(ast::IfElse& e) {
		// TODO: This holds a list of if branches, and an optional else branch
		// I think this is where any if handling will have to be situated
		// We might be required to translate all raw `IfBranch` nodes into an `IfElse` node

		// TODO: The Kaleidoscope tutorial explicitly adds all of the basic blocks to the function's BB list. Why?
		auto curr_fn = builder.GetInsertBlock()->getParent();
		auto& context = state.getContext();

		auto merge_point = BasicBlock::Create(context, "merge", curr_fn);
		auto branch_value_storage = builder.CreateAlloca(Type::getInt32Ty(context));
		std::vector<BasicBlock*> bbs;

		// TODO: How do I handle if branches with no else (for the purpose of the value)
		// TODO: I want to push this portion into the `visitIfBranch` code but it doesn't type right now
		// We would need to be able to pass in `merge_point` and `branch_value_storage` (set as member?)
		// It would also be possible to run `setInsertPoint; CreateBr; setInsertPoint` afterwards (with a lot of work)
		for (auto& branch : e.elems) {
			auto then_block = BasicBlock::Create(context, "then", curr_fn);
			auto else_block = BasicBlock::Create(context, "else", curr_fn);
			builder.CreateCondBr(visitNode(*branch->test), then_block, else_block);
			
			// Visit the conditional's body, storing the body's value on the stack
			builder.SetInsertPoint(then_block);
			// TODO: Should the storage be volatile or not
			builder.CreateStore(visitNode(*branch->body), branch_value_storage, false);
			builder.CreateBr(merge_point);

			bbs.push_back(builder.GetInsertBlock());
			builder.SetInsertPoint(else_block);
		}

		// Codegen the else branch in the leftover basic block from the if series
		if (e.else_) {
			builder.CreateStore(visitNode(*e.else_), branch_value_storage);
			builder.CreateBr(merge_point);
		}

		// Create the next block
		// NOTE: The last `BasicBlock` of the if series will be empty if there is no else statement (OK?)
		builder.CreateBr(merge_point);
		builder.SetInsertPoint(merge_point);

		// TODO: Create all the phi nodes
		// Looks like I'll have to know which values need "phi" before-hand
		
		// Load the value of the executed branch for future use
		codegen = builder.CreateLoad(branch_value_storage);
	}

	//
	// Statements
	//
	void LlvmIrGenerator::visitVarAssign(ast::VarAssign& v) {
		v.expr->accept(*this);
		v.name->accept(*this);
		codegen = nullptr;
	}

	//
	// Expressions
	//
	void LlvmIrGenerator::visitInAssign(ast::InAssign& a) {
		// TODO: Figure out how to handle variables in llvm ir
	}
	void LlvmIrGenerator::visitBinOpCall(ast::BinOpCall& b) {
		if (b.op == "=") {
			auto rhs = visitNode(*b.rhs);

			// TODO: Rewrite with a flat symbol table
			auto lhs = dynamic_cast<ast::Variable*>(b.lhs.get());
			auto[def_table, iter] = analysis::lookup(arena, current, *lhs->name);
			auto& path_part = **iter;
			auto variable = arena[def_table].get(path_part.name, nullptr, path_part.loc, path_part.ssa_index);

			auto& var = std::get<ref_t<analysis::SymbolInfo>>(*variable).get();
			if (var.storage) {
				codegen = builder.CreateStore(rhs, var.storage);
			}

			return;
		}

		auto lhs = visitNode(*b.lhs);
		auto rhs = visitNode(*b.rhs);

		if (b.op == "+") {
			codegen = builder.CreateAdd(lhs, rhs);

		} else if (b.op == "-") {
			codegen = builder.CreateSub(lhs, rhs);

		} else if (b.op == "*") {
			codegen = builder.CreateMul(lhs, rhs);

		} else if (b.op == "/") {
			// I'm assuming this means "create signed division"
			codegen = builder.CreateSDiv(lhs, rhs);

		} else if (b.op == "==") {
			codegen = builder.CreateICmpEQ(lhs, rhs);

		} else if (b.op == "!=") {
			codegen = builder.CreateICmpNE(lhs, rhs);

		} else if (b.op == "<") {
			codegen = builder.CreateICmpSLT(lhs, rhs);

		} else if (b.op == "<=") {
			codegen = builder.CreateICmpSLE(lhs, rhs);

		} else if (b.op == ">") {
			codegen = builder.CreateICmpSGT(lhs, rhs);

		} else if (b.op == ">=") {
			codegen = builder.CreateICmpSGE(lhs, rhs);

		} else if (b.op == "%") {
			// TODO: How to represent modulo in llvm ir?

		} else if (b.op == "&&") {
			// TODO: Does llvm implement lazy eval for &&/||
			codegen = builder.CreateAnd(lhs, rhs);

		} else if (b.op == "||") {
			// TODO: Does llvm implement lazy eval for &&/||
			codegen = builder.CreateOr(lhs, rhs);
		}
	}
	void LlvmIrGenerator::visitUnOpCall(ast::UnOpCall& u) {
		auto expr = visitNode(*u.expr);

		if (auto& op = u.op->name; op == "-") {
			codegen = builder.CreateNeg(expr);

		} else if (op == "!") {
			//auto comp_val = builder.CreateICmp(ICmpInst::ICMP_SGT, expr, Zero)
			codegen = builder.CreateNot(expr);
		}
	}
	void LlvmIrGenerator::visitFnCall(ast::FnCall& f) {
		auto fn_name = util::view_as<ast::Variable>(f.callee);
		if (!fn_name) {
			state.log(compiler::ID::err, "Calling non-var-bound functions is currently not supported <at {}>", f.loc);
			return;
		}

		// Extract the function object from llvm and perform some basic checking
		auto& func_name = fn_name->name->elems.back()->name.get();
		auto fn = translationUnit->getFunction(func_name);
		if (!fn) {
			state.log(compiler::ID::err, "Attempt to call unknown function {} <at {}>", func_name, f.loc);
			return;
		}

		// TODO: This isn't how spero handles too "few" arguments
		if (fn->arg_size() != f.arguments->elems.size()) {
			state.log(compiler::ID::err, "Incorrect number of arguments passed to function <at {}>", f.loc);
			return;
		}

		// Evaluate all of the arguments
		std::vector<Value*> args;
		for (auto& arg : f.arguments->elems) {
			args.push_back(visitNode(*arg));
		}

		// And create the call instruction
		codegen = builder.CreateCall(fn, args);
	}

}