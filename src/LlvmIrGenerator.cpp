#include "codegen/LlvmIrGenerator.h"

#include "util/parser.h"
#include "util/ranges.h"

namespace spero::compiler::gen {
	using namespace llvm;
	
	LlvmIrGenerator::LlvmIrGenerator(std::unique_ptr<llvm::Module> mod, analysis::SymArena& arena, CompilationState& state)
		: state{ state }, context{ state.getContext() }, arena{ arena }, translation_unit{ mod ? std::move(mod) : std::make_unique<llvm::Module>("speroc", state.getContext()) }, builder{ state.getContext() }
	{}

	LlvmIrGenerator::LlvmIrGenerator(analysis::SymArena& arena, CompilationState& state) : LlvmIrGenerator{ nullptr, arena, state } {}

	std::unique_ptr<llvm::Module> LlvmIrGenerator::finalize() {
		return std::move(translation_unit);
	}

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
	void LlvmIrGenerator::visitBool(ast::Bool& b) {
		// TODO: How to effectively represent boolean values in llvm ir ???
		int llvm_bool_val = b.val ? -1 : 0;
		codegen = llvm::ConstantInt::get(context, APInt(sizeof(llvm_bool_val) * 8, llvm_bool_val, true));
	}
	void LlvmIrGenerator::visitByte(ast::Byte& b) {
		codegen = ConstantInt::get(context, APInt(sizeof(b.val) * 8, b.val, false));
	}
	void LlvmIrGenerator::visitFloat(ast::Float& f) {
		codegen = ConstantFP::get(context, APFloat(f.val));
	}
	void LlvmIrGenerator::visitInt(ast::Int& i) {
		codegen = ConstantInt::get(context, APInt(sizeof(i.val) * 8, i.val, true));
	}
	void LlvmIrGenerator::visitChar(ast::Char& c) {
		codegen = ConstantInt::get(context, APInt(8, c.val, false));
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
		auto fn = translation_unit->getFunction(f.name->get());
		if (!fn) {
			// TODO: Introduce a translation from spero::Type to llvm::Type (requires spero::Type)
			std::vector<Type*> arg_types{ f.args.size(), Type::getInt32Ty(context) };
			auto fn_type = FunctionType::get(Type::getInt32Ty(context), arg_types, false);

			fn = Function::Create(fn_type, Function::ExternalLinkage, f.name->get(), translation_unit.get());

			// TODO: This introduces bugs if the defining code, uses different names
			// However, I feel we should already handle this during the initial language checks
			auto old_codegen = codegen;
			for (auto& arg : fn->args()) {
				codegen = &arg;
				visitArgument(*f.args[arg.getArgNo()]);
			}
			codegen = old_codegen;
		}

		if (!fn->empty()) {
			state.log(ID::err, "Function {} already has llvm definition <at {}>", f.name->get(), f.loc);
			return;
		}

		// Create the basic blocks
		auto entry_block = BasicBlock::Create(context, "entry", fn);
		builder.SetInsertPoint(entry_block);

		// TODO: Codegen initialisation code
		// Particularly for arguments which are marked as "mut"

		// TODO: Map function argument to symbol table
		if (auto retval = visitNode(*f.body)) {
			// TODO: Codegen cleanup code (this'll probably be pushed into the block)
			builder.CreateRet(retval);
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
		auto& path_part = v.name->elems.back();
		auto nvar = arena[*v.def_table].get(path_part->name, nullptr, path_part->loc, path_part->ssa_index);

		auto& symbol = std::get<ref_t<analysis::SymbolInfo>>(*nvar);
		if (symbol.get().storage) {
			auto storage = symbol.get().storage;
			if (!isa<Argument>(storage)) {
				codegen = builder.CreateLoad(storage);
			} else {
				codegen = storage;
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
						*translation_unit, Type::getInt32Ty(context), a.is_mut,
						GlobalVariable::ExternalLinkage, initializer, a.var->name.get()
					);
					// TODO: What do I do if the "initializer" isn't considered to be constant?
					symbol.storage = storage;
					//break;
				}
				case analysis::ScopingContext::SCOPE: {
					auto storage = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, a.var->name.get());
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
	// Decorations
	//
	void LlvmIrGenerator::visitArgument(ast::Argument& a) {
		// This function is expected to be called only in the context of `visitFunction`
		// Where the `codegen` member is set to the `llvm::Argument` of this variable
		if (auto arg = dyn_cast_or_null<Argument>(codegen)) {
			arg->setName(a.name->name.get());

			opt_t<size_t> ssa_index = std::nullopt;
			auto nvar = arena[current].get(a.name->name, nullptr, a.loc, ssa_index);

			// We can assume that the symbol exists if we get to this point (VarDeclPass would automatically insert it)
			std::get<ref_t<analysis::SymbolInfo>>(*nvar).get().storage = arg;

		} else {
			state.log(ID::err, "Attempt to visit arg {} with a non llvm::Argument* previous value <at {}>", a.name->name, a.loc);
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

		auto merge_point = BasicBlock::Create(context, "merge", curr_fn);
		std::vector<BasicBlock*> bbs;

		// Create the location to save the branch value
		// NOTE: We only create a value if there is an else branch
		// TODO: Only create this value if some code utilizes the value of the branch???s
		AllocaInst* branch_value_storage = nullptr;
		if (e.else_) {
			branch_value_storage = builder.CreateAlloca(Type::getInt32Ty(context));
		}

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
			if (branch_value_storage) {
				// TODO: Should the storage be volatile or not
				builder.CreateStore(visitNode(*branch->body), branch_value_storage, false);
			}
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
		// TODO: Figure out whether I need phi nodes (variables are just pushed on the stack)
		
		// Load the value of the executed branch for future use
		if (branch_value_storage) {
			codegen = builder.CreateLoad(branch_value_storage);
		}
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
	void LlvmIrGenerator::visitInAssign(ast::InAssign& in) {
		auto parent_scope = current;
		current = *in.binding;

		visitVarAssign(*in.bind);
		in.expr->accept(*this);

		current = parent_scope;
	}
	void LlvmIrGenerator::visitBinOpCall(ast::BinOpCall& b) {
		if (b.op == "=") {
			auto rhs = visitNode(*b.rhs);

			// TODO: Rewrite with a flat symbol table
			auto lhs = dynamic_cast<ast::Variable*>(b.lhs.get());
			auto& path_part = lhs->name->elems.back();
			auto variable = arena[*lhs->def_table].get(path_part->name, nullptr, path_part->loc, path_part->ssa_index);

			auto& var = std::get<ref_t<analysis::SymbolInfo>>(*variable).get();
			if (var.storage) {
				codegen = builder.CreateStore(rhs, var.storage);
			}

			return;
		}

		auto lhs = visitNode(*b.lhs);
		auto rhs = visitNode(*b.rhs);

		// Ensure the llvm values have the same types
		// TODO: Come up with a better system for this
		auto int32_type = Type::getInt32Ty(context);
		if (lhs->getType() != int32_type) {
			lhs = builder.CreateIntCast(lhs, int32_type, true);
		}
		if (rhs->getType() != int32_type) {
			rhs = builder.CreateIntCast(rhs, int32_type, true);
		}

		if (b.op == "+") {
			codegen = builder.CreateAdd(lhs, rhs);

		} else if (b.op == "-") {
			codegen = builder.CreateSub(lhs, rhs);

		} else if (b.op == "*") {
			codegen = builder.CreateMul(lhs, rhs);

		} else if (b.op == "/") {
			// I'm assuming this means "create signed division"
			codegen = builder.CreateSDiv(lhs, rhs);

		} else if (b.op == "%") {
			codegen = builder.CreateSRem(lhs, rhs);

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
			// TODO: Implement this correctly (but how?)
			//auto comp_val = builder.CreateICmp(ICmpInst::ICMP_SGT, expr, Zero)
			codegen = builder.CreateNot(expr);
		}
	}
	void LlvmIrGenerator::visitFnCall(ast::FnCall& f) {
		auto fn_name = util::viewAs<ast::Variable>(f.callee);
		if (!fn_name) {
			state.log(compiler::ID::err, "Calling non-var-bound functions is currently not supported <at {}>", f.loc);
			return;
		}

		// Extract the function object from llvm and perform some basic checking
		auto& func_name = fn_name->name->elems.back()->name.get();
		auto fn = translation_unit->getFunction(func_name);
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
		// TODO: How do I handle "self" arguments (for OOP) when they are needed?
		std::vector<Value*> args;
		for (auto& arg : f.arguments->elems) {
			args.push_back(visitNode(*arg));
		}

		// And create the call instruction
		codegen = builder.CreateCall(fn, args);
	}

}
