#include "codegen/AsmGenerator.h"
#include "util/parser.h"

using namespace asmjit;

namespace spero::compiler::gen {
	AsmGenerator::AsmGenerator(compiler::CompilationState& state) : emit{}, state{ state } {}

	Assembler AsmGenerator::get() {
		emit.setAllocatedStack(globals.numVariables() * 4);
		return std::move(emit);
	}

	void AsmGenerator::assign(std::string& var, bool is_mut, Location loc) {
		auto variable = current->getVar(var);
		
		if (!variable) {
			int off = -4 * (current->numVariables() + 1) - current->ebp_offset;
			current->insert(var, analysis::VarData{ off, loc, is_mut });
			emit.push(x86::eax);

		} else if (variable->get().is_mut) {
			emit.mov(x86::ptr(x86::ebp, variable->get().off), x86::eax);

		} else {
			state.log(ID::err, "Attempt to reassign immutable variable `{}` <at {}>", var, loc);
		}
	}

	//
	// Base Nodes
	//
	void AsmGenerator::visit(ast::Ast&) {}


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
	void AsmGenerator::visitBlock(ast::Block& b) {
		// Reserve stack space for local variables
		//emit.add(4 * b.locals.getCount(), x86::rsp);

		// Initialize current
		b.locals.setParent(current, true);
		current = &b.locals;

		// Emit the code for the function body
		for (auto& stmt : b.elems) {
			stmt->accept(*this);
		}

		// Very basic stack cleanup code (just pop all the variables off the stack)
		emit.popWords(current->numVariables());				// pop n bytes 
		current = current->getParent();
	}


	//
	// Names
	//
	// TODO: Generalize
	std::optional<ref_t<analysis::VarData>> AsmGenerator::scopedAccess(ast::Path& var) {
		auto* scope = current;
		auto name = var.elems.back()->name;

		while (scope) {
			if (scope->exists(name)) {
				return scope->getVar(name);
			}

			scope = scope->getParent();
		}

		return {};
	}

	void AsmGenerator::visitVariable(ast::Variable& v) {
		auto loc = scopedAccess(*v.name);

		if (!loc) {
			state.log(ID::err, "Attempt to use variable `{}` before it was declared <at {}>", v.name->elems.back()->name, v.loc);
			return;
		}

		emit.mov(x86::eax, x86::ptr(x86::ebp, loc->get().off));
	}

	void AsmGenerator::visitAssignName(ast::AssignName& n) {
		assign(n.var->name, n.is_mut, n.loc);
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


	//
	// Statements
	//
	void AsmGenerator::visitVarAssign(ast::VarAssign& v) {
		// if the body is a function
		// TODO: Adding support for functions may require moving this to a separate stage
		// TODO: Type checking will definitely require additional stages
		if (util::is_type<ast::Function>(v.expr)) {
			// Print out function datace
			emit.write(".def _main");		// main is 64-bit entrypoint
			emit.writef(".scl %d", 2);
			emit.writef(".type %d", 32);
			emit.write(".endef");
			emit.write(".globl _main");
			emit.writef(".p2align %d, %x", 4, 0x90);
			//emit.write(".type _main, @function");

			emit.bind(emit.newNamedLabel("_main"));

			v.expr->accept(*this);

		} else {
			// TODO: Figure out a way to pass the mutability of the expression on to the 'name'
			v.expr->accept(*this);					// Push the expression value onto the stack
			v.name->is_mut = v.expr->is_mut;
			v.name->accept(*this);
		}
	}


	//
	// Expressions
	//
	void AsmGenerator::visitInAssign(ast::InAssign& in) {
		// Setup the symbol table to prevent the context from leaking
		analysis::SymTable tmp{};
		tmp.setParent(current, true);
		current = &tmp;

		// Run through the assignment and expression
		in.bind->accept(*this);
		in.expr->accept(*this);

		// Pop off the symbol table
		emit.popWords(current->numVariables());
		current = current->getParent();
	}

	void AsmGenerator::visitFunction(ast::Function& f) {
		// Print function enter code
		// This is getting the 'main' label instead
		emit.bind(emit.newNamedLabel("LFB0"));
		emit.push(x86::ebp);
		emit.mov(x86::ebp, x86::esp);

		// Print the body
		f.body->accept(*this);

		// Print function tail/endlog
		emit.leave();
		emit.ret();
		emit.bind(emit.newNamedLabel("LFE0"));
	}

	void AsmGenerator::visitBinOpCall(ast::BinOpCall& b) {
		// TODO: Abstract to enable '*=' behavior and fallback if possible (analysis?)
		if (b.op == "=") {
			auto lhs = dynamic_cast<ast::Variable*>(b.lhs.get());

			// Ensure that the left-hand side is a variable node
			if (!lhs) {
				state.log(ID::err, "Attempt to reassign a non-variable value <at {}>", lhs->loc);
				return;
			}

			// TODO: Replace with a better system for detecting this case (will handle in assignment/access rewrite)
			if (!current->exists(lhs->name->elems.back()->name)) {
				state.log(ID::err, "Attempt to reassign a non-declared variable `{}` <at {}>", lhs->name->elems.back()->name, lhs->loc);
				return;
			}

			// Evaluate the value and perform the assignment
			b.rhs->accept(*this);
			// TODO: Add in mutability checks
				// NOTE: These ones have to be mutable (in order to pass)
			assign(lhs->name->elems.back()->name, true, b.loc);

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
		auto& op = u.op->name;
		if (op == "-") {
			emit.neg(x86::eax);

		} else if (op == "!") {
			// Emit a 'test' if the 'zero flag' isn't set
			// TODO: See if I can optimize this out if the 0 flag already set
			emit.test(x86::eax, x86::eax);
			emit.setz(x86::eax);
		}
	}

}
