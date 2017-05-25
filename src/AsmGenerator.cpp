#include "codegen/AsmGenerator.h"
#include "parser/utils.h"

namespace spero::compiler::gen {
	AsmGenerator::AsmGenerator(std::ostream& s) : emit{ s } {}
	

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


	// Types


	// Decorations


	// Control
	void AsmGenerator::acceptBlock(ast::Block& b) {
		for (auto& stmt : b.elems) {
			stmt->visit(*this);
		}
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
			emit.setz(eax);
			emit.popByte(1);

		} else if (b.op == "<") {
			emit.cmp(eax, esp.at());
			emit.setl(eax);
			emit.popByte(1);
		
		} else if (b.op == ">") {
			emit.cmp(esp.at(), eax);
			emit.setl(eax);
			//emit.cmp(eax, esp.at());
			//emit.setg(eax);
			emit.popByte(1);

		} else if (b.op == "!=") {
			emit.cmp(eax, esp.at());
			emit.setnz(eax);
			emit.popByte(1);

		} else if (b.op == "<=") {
			emit.cmp(eax, esp.at());
			emit.setle(eax);
			emit.popByte(1);

		} else if (b.op == ">=") {
			emit.cmp(esp.at(), eax);
			emit.setle(eax);
			emit.popByte(1);
		}
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
		// Print out function data
		emit.write("\t.p2align 4, 0x90");
		emit.write("\t.globl _main\n");
		emit.write("\t.def _main; .scl 2; .type 32; .endef\n");
		emit.label("_main");

		v.expr->visit(*this);

		// Print function ident information
		emit.write("\t.ident \"speroc: 0.0.15 (Windows 2017)\"");
	}
}