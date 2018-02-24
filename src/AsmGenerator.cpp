#include "codegen/AsmGenerator.h"
#include "util/parser.h"

using namespace asmjit;

namespace spero::compiler::gen {
	AsmGenerator::AsmGenerator(compiler::CompilationState& state) : emit{}, state{ state } {}

	Assembler AsmGenerator::get() {
		emit.setAllocatedStack(globals.getCount() * 4);
		return std::move(emit);
	}

	void AsmGenerator::assign(std::string& var, bool force_curr, Location loc) {
		auto variable = current->getVar(var, force_curr);

		if (force_curr && !variable) {
			int off = -4 * (current->getCount() + 1);
			current->insert(var, off, loc);

			emit.push(x86::eax);

		} else if (variable) {
			auto dst = x86::ptr(x86::ebp, variable.value());
			emit.mov(dst, x86::eax);
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

		// NOTE: clang emits `xor eax, eax` for false
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
		//_Register esp{ "esp" };
		//emit.add(4 * b.locals.getCount(), esp);

		// Initialize current
		b.locals.setParent(current, true);
		current = &b.locals;

		// Emit the code for the function body
		for (auto& stmt : b.elems) {
			stmt->accept(*this);
		}

		// Very basic stack cleanup code (just pop all the variables off the stack)
		emit.popWords(current->getCount());				// pop n bytes 
		current = current->getParent();
	}


	//
	// Names
	//
	void AsmGenerator::visitVariable(ast::Variable& v) {
		auto var = v.name->elems.back()->name;
		auto loc = current->getVar(var);

		if (!loc) {
			state.log(ID::err, "Attempt to use variable `{}` before it was declared <at {}>", var, v.loc);
			return;
		}

		emit.mov(x86::eax, x86::ptr(x86::ebp, loc.value()));
	}

	void AsmGenerator::visitAssignName(ast::AssignName& n) {
		assign(n.var->name, true, n.loc);
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
			emit.write(".global _main");
			emit.write(".def _main");
			emit.write(".scl 2");
			emit.writef(".scl %d", 2);
			emit.write(".type %d", 32);
			emit.write(".endef");

			emit.newNamedLabel("_main");

			v.expr->accept(*this);

			// Print function ident information
			emit.writef(".ident \"%s: %d.%d.%d (%s)\"", "speroc", 0, 0, 15, "Windows 2018");

		} else {
			v.expr->accept(*this);					// Push the expression value onto the stack
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
		emit.popWords(current->getCount());
		current = current->getParent();
	}

	void AsmGenerator::visitFunction(ast::Function& f) {
		// Print function enter code
		emit.newNamedLabel("LFB0");
		emit.push(x86::ebp);
		emit.mov(x86::ebp, x86::esp);

		// Print the body
		f.body->accept(*this);

		// Print function tail/endlog
		emit.leave();
		emit.ret();
		emit.newNamedLabel("LFE0");
	}

	void AsmGenerator::visitBinOpCall(ast::BinOpCall& b) {
		// TODO: Abstract to enable '*=' behavior and fallback if possible (analysis?)
		if (b.op == "=") {
			auto _lhs = dynamic_cast<ast::Variable*>(b.lhs.get());

			// Ensure that the left-hand side is a variable node
			if (!_lhs) {
				state.log(ID::err, "Attempt to reassign a non-variable value");
				return;
			}

			// Evaluate the value and perform the assignment
			b.rhs->accept(*this);
			assign(_lhs->name->elems.back()->name, false, b.loc);

		} else {
			// Evaluate the left side and store it on the stack
			b.lhs->accept(*this);
			emit.push(x86::eax);

			// Evaluate the right side
			b.rhs->accept(*this);

			// Perform the operator call
			if (b.op == "+") {
				emit.add(x86::eax, x86::ptr(x86::esp));
				emit.popWords(1);

			} else if (b.op == "-") {
				emit.sub(x86::ptr(x86::esp), x86::eax);
				emit.pop(x86::eax);

			} else if (b.op == "*") {
				emit.imul(x86::eax, x86::ptr(x86::esp));
				emit.popWords(1);

			} else if (b.op == "/") {
				emit.cdq();
				emit.idiv(x86::ptr(x86::esp));
				emit.popWords(1);

			} else if (b.op == "==") {
				emit.cmp(x86::ptr(x86::esp), x86::eax);
				emit.setz(x86::al);
				emit.popWords(1);

			} else if (b.op == "!=") {
				emit.cmp(x86::ptr(x86::esp), x86::eax);
				emit.setnz(x86::al);
				emit.popWords(1);

			} else if (b.op == "<") {
				// TODO: Implement

			} else if (b.op == "<=") {
				// TODO: Implement

			} else if (b.op == ">") {
				// TODO: Implement

			} else if (b.op == ">=") {
				// TODO: Implement

			} else if (b.op == "%") {
				emit.cdq();
				emit.idiv(x86::ptr(x86::esp));

				emit.mov(x86::edx, x86::eax);
				emit.popWords(1);
			}
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
