#include "codegen/AsmGenerator.h"

namespace spero::compiler::codegen {
	AsmGenerator::AsmGenerator(std::ostream& s) : out{ s } {}

	void AsmGenerator::accept(ast::Ast&) {}

	void AsmGenerator::acceptInt(ast::Int& i) {
		out << "\tmov $" << i.val << ", %eax\n";
	}

	void AsmGenerator::acceptFnBody(ast::FnBody& f) {
		out << "\t.p2align 4, 0x90\n"
			<< "\t.globl _main\n"
			<< "\t.def _main; .scl 2; .type 32; .endef\n"
			<< "_main:\n"
			<< "LFB0:\n"
			<< "\tpush %ebp\n"
			<< "\tmov %esp, %ebp\n"
			<< "\tpush %eax\n";

		// Print the int (what does the fn have?)
		f.body->visit(*this);

		// Print function tail/endlog
		out << "\tleave\n"
			<< "\tret\n"
			<< "LFE0:\n"
			<< "\t.ident \"speroc: 0.0.1 (Windows 2016)\"";
	}

	void AsmGenerator::acceptVarAssign(ast::VarAssign& v) {
		v.expr->visit(*this);
	}
}