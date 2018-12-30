#include "codegen/AsmGenerator.h"

#include "util/parser.h"
#include "util/ranges.h"

using namespace asmjit;

namespace spero::compiler::gen {
	using analysis::GLOBAL_SYM_INDEX;

	AsmGenerator::AsmGenerator(analysis::SymArena& arena, compiler::CompilationState& state) : arena{ arena }, state{ state }, reporter{ state } {
		emit.setErrorHandler(&reporter);
		
		// TODO: Quick hack to enable 'batched' pushing of variables
		// Because the global scope isn't a table, the batch push didn't occur
		// NOTE: Will be fixed when I move to a static/global allocation scheme
		//emit.pushWords(numVars(arena, GLOBAL_SYM_INDEX) - arena[GLOBAL_SYM_INDEX].exists("_main"));
	}

	Assembler AsmGenerator::finalize() {
		//auto num_vars = numVars(arena, GLOBAL_SYM_INDEX) - arena[GLOBAL_SYM_INDEX].exists("_main");
		//emit.setAllocatedStack(num_vars * 4);
		return std::move(emit);
	}

	//
	// Literals
	//
	void AsmGenerator::visitBool(ast::Bool& b) {
		emit.mov(x86::eax, b.val);

		// NOTE: clang emits `xor rax, rax` for false
	}

	void AsmGenerator::visitByte(ast::Byte& b) {
		emit.xor_(x86::eax, x86::eax);

		// TODO: Fix this difficulty
		emit.mov(x86::eax, (unsigned int)b.val);
	}

	void AsmGenerator::visitFloat(ast::Float& f) {
		//
	}

	void AsmGenerator::visitInt(ast::Int& i) {
		emit.mov(x86::eax, i.val);
	}

	void AsmGenerator::visitChar(ast::Char& c) {
		emit.mov(x86::al, c.val);
	}


	//
	// Atoms
	//
	void AsmGenerator::visitTuple(ast::Tuple& t) {
		AsmGenerator::visitValExpr(t);

		for (auto&& elem : t.elems) {
			elem->accept(*this);
			emit.push(x86::eax);
		}
	}
	void AsmGenerator::visitBlock(ast::Block& b) {
		// Initialize scope entry code
		auto parent_scope = current;
		current = *b.locals;

		// TODO: Need to reintroduce argument handling
		//auto num_vars = numVars(arena, current);
		//emit.pushWords(num_vars - numArgs(arena, current));
		// TODO: Zero initialize the allocated stack space?

		// Emit the code for the function body
		AstVisitor::visitBlock(b);

		// Very basic stack cleanup code (just pop all the variables off the stack)
		//emit.popWords(num_vars);		// pop n bytes 
		current = parent_scope;
	}

	// NOTE: Doesn't work on local functions
	void AsmGenerator::visitFunction(ast::Function& f) {
		// Output assembly specific code
		if (!f.name) {
			// TODO: This is a temporary solution to prevent generation of code for local functions (in case I try it for now)
			state.log(compiler::ID::err, "Attempt to generate assembly for function without a name <at {}>", f.loc);
			return;
		}

		auto& name = f.name->get();
		emit.write((".def " + name).c_str());
		emit.writef(".scl %d", 2);
		emit.writef(".type %d", 32);
		emit.write(".endef");
		emit.write((".globl " + name).c_str());
		emit.writef(".p2align %d, %x", 4, 0x90);
		//emit.write(".type _main, @function");

		// Handle cases where the label was created during `visitFnCall` (in the case of use-before-dec)
		// TODO: Mangle the function label
		auto label = emit.labelByName(name.c_str());
		if (!label.isValid()) {
			label = emit.newNamedLabel(name.c_str());
		}
		emit.bind(label);

		// Setup the function
		FuncDetail func;
		// NOTE: Assume that we're always returning an int for now
		// Once we get to the assembly stage we should already have converted everyting to something like this anyways
		func.init(FuncSignatureT<int>(CallConv::kIdHost));

		FuncFrame ffi;
		ffi.init(func);
		ffi.setAllDirty();
		emit.emitProlog(ffi);
		emit.mov(x86::ebp, x86::esp);

		// Place function arguments where the function expects them to go
		// TODO: Fix this as it's extremely inefficient
		// NOTE: Block allocation knows how many "local" variables were function arguments
		const auto offset = f.args.size() + 4;
		for (auto& _ : f.args) {
			emit.push(x86::ptr(x86::esp, static_cast<int32_t>(offset * 4)));
		}

		// Perform codegen on the function
		AstVisitor::visitFunction(f);

		// Tear-down the function
		// NOTE: Currently all operations store their result in eax, so we don't need to worry about return stuff yet
		emit.popBytes(0);
		emit.emitEpilog(ffi);
	}


	//
	// Names
	//
	void AsmGenerator::visitVariable(ast::Variable& v) {
		auto[def_table, iter] = analysis::lookup(arena, current, *v.name);
		auto nvar = arena[def_table].get((**iter).name, nullptr, (**iter).loc, (**iter).ssa_index);

		std::visit([&](auto&& var) {
			if constexpr (std::is_same_v<std::decay_t<decltype(var)>, ref_t<analysis::SymbolInfo>>) {
				//auto loc = std::get<analysis::memory::Stack>(var.get().storage);
				//emit.mov(x86::eax, x86::ptr(x86::ebp, loc.ebp_offset));
			}
		}, *nvar);
	}

	void AsmGenerator::visitAssignName(ast::AssignName& n) {
		opt_t<size_t> ssa_index = std::nullopt;
		auto nvar = arena[current].get(n.var->name, nullptr, n.var->loc, ssa_index);
		if (nvar) {
			std::visit([&](auto&& var) {
				using VarType = std::decay_t<decltype(var)>;
				if constexpr (std::is_same_v<VarType, ref_t<analysis::SymbolInfo>>) {
					//if (std::holds_alternative<analysis::memory::Global>(var.get().storage) && dynamic_cast<ast::Function*>(var.get().definition)) {
						//return;
					//}

					// NOTE: A better approach for this would probably be to create an "arena" for allocating
						// The arena would provide the interface for pushing/accessing stuff in the memory region it defines
						// The ast/visitor/etc. only need to push/allocate bytes into the region
					//auto loc = std::get<analysis::memory::Stack>(var.get().storage);
					//emit.mov(x86::ptr(x86::ebp, loc.ebp_offset), x86::eax);
				}
				}, *nvar);
		}
	}


	//
	// Types
	//


	//
	// Decorations
	//


	//
	// Control
	//
	void AsmGenerator::visitIfBranch(ast::IfBranch& branch) {
		visitBranch(branch);

		branch.test->accept(*this);
		emit.cmp(x86::al, 0);

		auto after_label = emit.newLabel();
		emit.je(after_label);

		branch.body->accept(*this);
		emit.bind(after_label);
	}
	void AsmGenerator::visitIfElse(ast::IfElse& branches) {
		visitBranch(branches);

		auto end_label = emit.newLabel();
		for (auto& branch : branches.elems) {
			visitBranch(*branch);

			branch->test->accept(*this);
			emit.cmp(x86::al, 0);
			
			auto next_branch = emit.newLabel();
			emit.je(next_branch);

			branch->body->accept(*this);
			emit.jmp(end_label);

			emit.bind(next_branch);
		}

		if (branches.else_) {
			branches.else_->accept(*this);
		}

		emit.bind(end_label);
	}


	//
	// Statements
	//


	//
	// Expressions
	//
	void AsmGenerator::visitInAssign(ast::InAssign& in) {
		// Setup the symbol table to prevent the context from leaking
			// TODO: Could probably get a more efficient method by just adding a temporary key in the current table
			// TODO: Need to rewrite before 'variable pass' decoupling as this won't work in that framework
		auto parent_scope = current;
		current = *in.binding;

		/*auto num_vars = numVars(arena, current);
		emit.pushWords(num_vars);*/

		// Run through the assignment and expression
		in.bind->accept(*this);
		in.expr->accept(*this);

		// Pop off the symbol table
		//emit.popWords(num_vars);
		current = parent_scope;
	}

	void AsmGenerator::visitBinOpCall(ast::BinOpCall& b) {
		if (b.op == "=") {
			// Evaluate the value into the appropriate registers
			b.rhs->accept(*this);

			auto* lhs = dynamic_cast<ast::Variable*>(b.lhs.get());
			auto[def_table, iter] = analysis::lookup(arena, current, *lhs->name);
			auto variable = arena[def_table].get((**iter).name, nullptr, (**iter).loc, (**iter).ssa_index);
			auto& var = std::get<ref_t<analysis::SymbolInfo>>(*variable).get();
			//auto& loc = std::get<analysis::memory::Stack>(var.storage);

			//emit.mov(x86::ptr(x86::ebp, loc.ebp_offset), x86::eax);

		} else {
			// Evaluate the left side and store it on the stack
			b.lhs->accept(*this);
			emit.push(x86::eax);

			auto numPushedWords = 1;

			// Evaluate the right side
			b.rhs->accept(*this);

			// Perform the operator call
			if (b.op == "+") {
				emit.add(x86::eax, x86::ptr(x86::esp));

			} else if (b.op == "-") {
				emit.sub(x86::ptr(x86::esp), x86::eax);
				emit.mov(x86::eax, x86::ptr(x86::esp));

			} else if (b.op == "*") {
				emit.imul(x86::eax, x86::ptr(x86::esp));

			} else if (b.op == "/") {
				emit.xchg(x86::ptr(x86::esp), x86::eax);
				emit.pop(x86::ecx);
				emit.cdq();
				emit.idiv(x86::ecx);

				numPushedWords = 0;

			} else if (b.op == "==") {
				emit.cmp(x86::ptr(x86::esp), x86::eax);
				emit.mov(x86::eax, 0);
				emit.setz(x86::al);

			} else if (b.op == "!=") {
				emit.cmp(x86::ptr(x86::esp), x86::eax);
				emit.mov(x86::eax, 0);
				emit.setnz(x86::al);

			} else if (b.op == "<") {
				emit.cmp(x86::ptr(x86::esp), x86::eax);
				emit.mov(x86::eax, 0);
				emit.setc(x86::al);

			} else if (b.op == "<=") {
				emit.cmp(x86::eax, x86::ptr(x86::esp));
				emit.mov(x86::eax, 0);
				emit.setc(x86::al);
				emit.xor_(x86::eax, 1);

			} else if (b.op == ">") {
				emit.cmp(x86::eax, x86::ptr(x86::esp));
				emit.mov(x86::eax, 0);
				emit.setc(x86::al);

			} else if (b.op == ">=") {
				emit.cmp(x86::ptr(x86::esp), x86::eax);
				emit.mov(x86::eax, 0);
				emit.setc(x86::al);
				emit.xor_(x86::eax, 1);

			} else if (b.op == "%") {
				emit.xchg(x86::ptr(x86::esp), x86::eax);
				emit.pop(x86::ecx);
				emit.cdq();
				emit.idiv(x86::ecx);
				emit.mov(x86::eax, x86::edx);

				numPushedWords = 0;

			} else if (b.op == "&&") {
				emit.and_(x86::eax, x86::ptr(x86::esp));
				// TODO: Implement lazy evaluation (needed at this stage?)

			} else if (b.op == "||") {
				emit.or_(x86::eax, x86::ptr(x86::esp));
				// TODO: Implement lazy evaluation (needed at this stage?)

			}

			emit.popWords(numPushedWords);
		}
	}

	void AsmGenerator::visitUnOpCall(ast::UnOpCall& u) {
		// Evaluate the expression
		u.expr->accept(*this);

		// Perform the operator call
		// TODO: Perform type-based function lookup
		if (auto& op = u.op->name; op == "-") {
			emit.neg(x86::eax);

		} else if (op == "!") {
			// Emit a 'test' if the 'zero flag' isn't set
			// TODO: See if I can optimize this out if the 0 flag already set
			emit.test(x86::eax, x86::eax);
			emit.setz(x86::eax);
		}
	}

	void AsmGenerator::visitFnCall(ast::FnCall& f) {
		visitValExpr(f);

		f.arguments->accept(*this);

		// Push arguments on the stack
		if (!util::is_type<ast::Variable>(f.callee)) {
			state.log(compiler::ID::err, "Calling non-var-bound functions is currently not supported <at {}>", f.loc);
			return;
		}

		auto* fn = util::view_as<ast::Variable>(f.callee);

		// TODO: This requires having the declaration before the call site
		auto label = emit.labelByName(fn->name->elems.back()->name.get().c_str());
		if (!label.isValid()) {
			label = emit.newNamedLabel(fn->name->elems.back()->name.get().c_str());
		}

		emit.call(label);
		emit.popWords(f.arguments->elems.size());
	}

}

namespace spero::compiler::ast {
	// NOTE: Even though this method is marked `abstract`, we have to give a definition because it's a constructor
	AstVisitor::~AstVisitor() {}
}
