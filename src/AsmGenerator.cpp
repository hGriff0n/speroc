#include "codegen/AsmGenerator.h"
#include "parser/utils.h"

namespace spero::compiler::gen {
	AsmGenerator::AsmGenerator(std::ostream& s) : assm{ s } {}
	

	// Base Nodes
	void AsmGenerator::accept(ast::Ast&) {}


	// Literals
	void AsmGenerator::acceptBool(ast::Bool& b) {
		Register eax{ "eax" };
		if (b.val)
			assm.mov(b.val, eax);

		else
			assm._xor(eax, eax);
	}

	void AsmGenerator::acceptByte(ast::Byte& b) {
		Register eax{ "eax" };
		assm._xor(eax, eax);
		assm.mov(b.val, eax);
	}

	void AsmGenerator::acceptFloat(ast::Float& f) {
		
	}

	void AsmGenerator::acceptInt(ast::Int& i) {
		Register eax{ "eax" };
		assm.mov(i.val, eax);
	}

	void AsmGenerator::acceptChar(ast::Char& c) {
		Register al{ "al" };
		assm.mov(c.val, al);
	}


	// Names


	// Types


	// Decorations


	// Control
	void AsmGenerator::acceptBlock(ast::Block& b) {
		for (auto& stmt : b.elems)
			stmt->visit(*this);
	}


	// Statements
	void AsmGenerator::acceptFnBody(ast::FnBody& f) {
		Register ebp{ "ebp" }, esp{ "esp" }, eax{ "eax" };

		// Print function enter code
		assm.emitLabel("LFB0");
		assm.push(ebp);
		assm.mov(esp, ebp);
		assm.push(eax);

		// Print the body
		f.body->visit(*this);

		// Print function tail/endlog
		assm.leave();
		assm.ret();
		assm.emitLabel("LFE0");
	}

	void AsmGenerator::acceptBinOpCall(ast::BinOpCall& b) {
		Register eax{ "eax" };
		Register esp{ "esp" };

		// Evaluate the left side and store it on the stack
		b.lhs->visit(*this);
		assm.push(eax);

		// Evaluate the right side
		b.rhs->visit(*this);

		// Perform the operator call
		if (b.op == "+") {
			assm.add(esp.at(), eax);
			assm.add(4, esp);

		} else if (b.op == "-") {
			assm.sub(eax, esp.at());
			assm.pop(eax);

		} else if (b.op == "*") {
			assm.imul(esp.at(), eax);
			assm.add(4, esp);

		} else if (b.op == "/") {
			assm.cdq();
			assm.idiv(esp.at(), eax);
			assm.add(4, esp);

		} else if (b.op == "==") {
			assm.cmp(eax, esp.at());
			assm.setz(eax);
			assm.add(4, esp);

		} else if (b.op == "<") {
			assm.cmp(eax, esp.at());
			assm.setl(eax);
			assm.add(4, esp);
		
		} else if (b.op == ">") {

		} else if (b.op == "!=") {
			
		} else if (b.op == "<=") {

		} else if (b.op == ">=") {

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
				assm.neg(eax);
				break;
			case ast::UnaryType::NOT:
				if (!assm.wasZeroSet())				// Insert a 'test' instruction if the 'zero flag' isn't set
					assm.test(eax, eax);

				assm.setz(eax);
		}
	}

	void AsmGenerator::acceptVarAssign(ast::VarAssign& v) {
		// Print out function data
		assm.emit("\t.p2align 4, 0x90");
		assm.emit("\t.globl _main\n");
		assm.emit("\t.def _main; .scl 2; .type 32; .endef\n");
		assm.emitLabel("_main");

		v.expr->visit(*this);

		// Print function ident information
		assm.emit("\t.ident \"speroc: 0.0.15 (Windows 2017)\"");
	}
}