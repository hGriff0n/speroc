#include "codegen/AsmGenerator.h"
#include "util/parser.h"

namespace spero::compiler::gen {
	AsmGenerator::AsmGenerator(std::ostream& s, compiler::CompilationState& state) : out{ s }, emit { s }, state{ state } {}
	
	void AsmGenerator::assign(std::string& var, bool force_curr, Location loc) {
		Register eax{ "eax" };
		auto variable = current->getVar(var, force_curr);

		if (force_curr && !variable) {
			int off = -4 * (current->getCount() + 1);
			current->insert(var, off, loc);

			emit.push(eax);

		} else if (variable) {
			Register ebp{ "ebp" };
			emit.mov(eax, ebp.at(variable.value()));
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
		Register eax{ "eax" };
		
		if (b.val) {
			emit.mov(b.val, eax);

		} else {
			emit._xor(eax, eax);
		}
	}

	void AsmGenerator::visitByte(ast::Byte& b) {
		Register eax{ "eax" };
		emit._xor(eax, eax);
		emit.mov(b.val, eax);
	}

	void AsmGenerator::visitFloat(ast::Float& f) {
		//
	}

	void AsmGenerator::visitInt(ast::Int& i) {
		Register eax{ "eax" };
		emit.mov(i.val, eax);
	}

	void AsmGenerator::visitChar(ast::Char& c) {
		Register al{ "al" };
		emit.mov(c.val, al);
	}


	//
	// Atoms
	//
	void AsmGenerator::visitBlock(ast::Block& b) {
		// Reserve stack space for local variables
		//Register esp{ "esp" };
		//emit.add(4 * b.locals.getCount(), esp);

		// Initialize current
		b.locals.setParent(current, true);
		current = &b.locals;

		// Emit the code for the function body
		for (auto& stmt : b.elems) {
			stmt->accept(*this);
		}

		// Very basic stack cleanup code (just pop all the variables off the stack)
		emit.popByte(current->getCount());
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

		Register eax{ "eax" };
		Register ebp{ "ebp" };
		emit.mov(ebp.at(loc.value()), eax);
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

			// Print out function data
			emit.write("\t.globl _main\n");
			emit.write("\t.def _main; .scl 2; .type 32; .endef\n");
			emit.label("_main");

			v.expr->accept(*this);

			// Print function ident information
			emit.write("\t.ident \"speroc: 0.0.15 (Windows 2017)\"");

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
		emit.popByte(current->getCount());
		current = current->getParent();
	}

	void AsmGenerator::visitFunction(ast::Function& f) {
		Register ebp{ "ebp" }, esp{ "esp" }, eax{ "eax" };

		// Print function enter code
		emit.label("LFB0");
		emit.push(ebp);
		emit.mov(esp, ebp);

		// Print the body
		f.body->accept(*this);

		// Print function tail/endlog
		emit.leave();
		emit.ret();
		emit.label("LFE0");
	}

	void AsmGenerator::visitBinOpCall(ast::BinOpCall& b) {
		Register eax{ "eax" };

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
			Register esp{ "esp" };
			Register al{ "al" };

			// Evaluate the left side and store it on the stack
			b.lhs->accept(*this);
			emit.push(eax);

			// Evaluate the right side
			b.rhs->accept(*this);

			// Perform the operator call
			if (b.op == "+") {
				emit.add(esp.at(), eax);
				emit.popByte(1);

			} else if (b.op == "-") {
				emit.sub(eax, esp.at());
				emit.pop(eax);

			} else if (b.op == "*") {
				emit.imul(esp.at(), eax);
				emit.popByte(1);

			} else if (b.op == "/") {
				emit.cdq();
				emit.idiv(esp.at(), eax);
				emit.popByte(1);

			} else if (b.op == "==") {
				emit.cmp(eax, esp.at());
				emit.setz(al);
				emit.popByte(1);

			} else if (b.op == "<") {
				emit.cmp(eax, esp.at());
				emit.setl(al);
				emit.popByte(1);

			} else if (b.op == ">") {
				emit.cmp(eax, esp.at());
				emit.setg(al);
				emit.popByte(1);

			} else if (b.op == "!=") {
				emit.cmp(eax, esp.at());
				emit.setnz(al);
				emit.popByte(1);

			} else if (b.op == "<=") {
				emit.cmp(eax, esp.at());
				emit.setle(al);
				emit.popByte(1);

			} else if (b.op == ">=") {
				emit.cmp(eax, esp.at());
				emit.setge(al);
				emit.popByte(1);

			}
		}
	}

	void AsmGenerator::visitUnOpCall(ast::UnOpCall& u) {
		// Evaluate the expression
		u.expr->accept(*this);

		Register eax{ "eax" };

		// Perform the operator call
		// TODO: Perform type-based function lookup
		auto& op = u.op->name;
		if (op == "-") {
			emit.neg(eax);

		} else if (op == "!") {
			// Emit a 'test' if the 'zero flag' isn't set
			if (!emit.zeroSet()) {
				emit.test(eax, eax);
			}

			emit.setz(eax);
		}
	}

}
