#include "codegen/AsmGenerator.h"
#include "util/parser.h"

namespace spero::compiler::gen {
	AsmGenerator::AsmGenerator(std::ostream& s, CompilationState& state) : out{ s }, emit { s }, state{ state } {}

	void AsmGenerator::loadVariable(ast::Variable& v) {
		auto var = v.name->toString();
		auto loc = current->getVar(var);

		if (!loc) {
			state.log(ID::err, "Attempt to use a variable before it was declared {}", CompilationState::location(v.loc));
			return;
		}

		Register ebp{ "ebp" };
		out << ebp.at(loc.value());
	}

	void AsmGenerator::performAssign(ast::AssignPattern& pat, bool force_curr_scope) {
		performAssign(dynamic_cast<ast::AssignName&>(pat).var->toString(), pat.loc, force_curr_scope);
	}

	void AsmGenerator::performAssign(std::string& var, ast::Location src, bool force_curr_scope) {
		auto loc = current->getVar(var, force_curr_scope);

		if (!loc) {
			current->insert(var, -4 * (current->getCount() + 2), src);

			Register eax{ "eax" };
			emit.push(eax);

		} else {
			Register ebp{ "ebp" };
			out << "\tmov %eax, " << ebp.at(loc.value()) << '\n';
		}
	}
	

	// Base Nodes
	void AsmGenerator::accept(ast::Ast&) {}


	// Literals
	void AsmGenerator::acceptBool(ast::Bool& b) {
		Register eax{ "eax" };
		
		if (b.val) {
			emit.mov(b.val, eax);

		} else {
			emit._xor(eax, eax);
		}
	}

	void AsmGenerator::acceptByte(ast::Byte& b) {
		Register eax{ "eax" };
		emit._xor(eax, eax);
		emit.mov(b.val, eax);
	}

	void AsmGenerator::acceptFloat(ast::Float& f) {
		//
	}

	void AsmGenerator::acceptInt(ast::Int& i) {
		Register eax{ "eax" };
		emit.mov(i.val, eax);
	}

	void AsmGenerator::acceptChar(ast::Char& c) {
		Register al{ "al" };
		emit.mov(c.val, al);
	}


	// Names
	void AsmGenerator::acceptVariable(ast::Variable& v) {
		Register eax{ "eax" };
		out << "\tmov ";
		loadVariable(v);
		out << ", " << eax << '\n';
	}


	// Types


	// Decorations


	// Control
	void AsmGenerator::acceptBlock(ast::Block& b) {
		// Reserve stack space for local variables
		//Register esp{ "esp" };
		//emit.add(4 * b.locals.getCount(), esp);
		
		// Initialize current
		b.locals.setParent(current, true);
		current = &b.locals;

		// Emit the code for the function body
		for (auto& stmt : b.elems) {
			stmt->visit(*this);
		}

		// Very basic stack cleanup code (just pop all the variables off the stack)
		emit.popByte(current->getCount());
		current = current->getParent();
	}


	// Statements
	void AsmGenerator::acceptFnBody(ast::FnBody& f) {
		Register ebp{ "ebp" }, esp{ "esp" }, eax{ "eax" };

		// Print function enter code
		emit.label("LFB0");
		emit.push(ebp);
		emit.mov(esp, ebp);
		emit.push(eax);

		// Print the body
		f.body->visit(*this);

		// Print function tail/endlog
		emit.leave();
		emit.ret();
		emit.label("LFE0");
	}

	void AsmGenerator::acceptBinOpCall(ast::BinOpCall& b) {
		Register eax{ "eax" };
		Register esp{ "esp" };
		Register al{ "al" };

		// Evaluate the left side and store it on the stack
		b.lhs->visit(*this);
		emit.push(eax);

		// Evaluate the right side
		b.rhs->visit(*this);

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

	void AsmGenerator::acceptReassign(ast::Reassign& r) {
		// Evaluate the operands
		r.val->visit(*this);
		performAssign(r.var->name->toString(), r.loc, false);
	}

	void AsmGenerator::acceptUnaryOpApp(ast::UnaryOpApp& u) {
		// Evaluate the expression
		u.expr->visit(*this);

		Register eax{ "eax" };

		// Perform the operator call
		// TODO: Perform type-based function lookup
		switch (u.op) {
			case ast::UnaryType::MINUS:
				emit.neg(eax);
				break;
			case ast::UnaryType::NOT:
				if (!emit.zeroSet()) {				// Insert a 'test' instruction if the 'zero flag' isn't set
					emit.test(eax, eax);
				}
				emit.setz(eax);
		}
	}

	void AsmGenerator::acceptVarAssign(ast::VarAssign& v) {
		// if the body is a function
		// TODO: Adding support for functions may require moving this to a separate stage
		// TODO: Type checking will definitely require additional stages
		if (util::is_type<ast::FnBody>(v.expr)) {

			// Print out function data
			emit.write("\t.globl _main\n");
			emit.write("\t.def _main; .scl 2; .type 32; .endef\n");
			emit.label("_main");

			v.expr->visit(*this);

			// Print function ident information
			emit.write("\t.ident \"speroc: 0.0.15 (Windows 2017)\"");

		} else {
			v.expr->visit(*this);					// Push the expression value onto the stack
			performAssign(*v.name, true);			// Store the variable at its location
		}
	}
}
