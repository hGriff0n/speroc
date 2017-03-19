#include "codegen/AsmGenerator.h"

namespace spero::compiler::codegen {
	AsmGenerator::AsmGenerator(std::ostream& s) : out{ s } {}

	void AsmGenerator::accept(ast::Ast&) {}

	void AsmGenerator::acceptBool(ast::Bool& b) {
		if (b.val)
			out << "\tmov $" << b.val << ", %al\n";

		else
			out << "\txor %eax, %eax\n";
	}

	void AsmGenerator::acceptByte(ast::Byte& b) {
		out << "\txor %eax, %eax\n\tmov $" << b.val << ", %eax\n";
	}

	void AsmGenerator::acceptFloat(ast::Float& f) {
		/* clang -O1
		.LCPI5_0:
			.quad   4601778099247172813     # double 0.45000000000000001
		ret_float():                          # @ret_float()
			movsd   xmm0, qword ptr [rip + .LCPI5_0] # xmm0 = mem[0],zero
			ret
		*/
	}

	void AsmGenerator::acceptInt(ast::Int& i) {
		out << "\tmov $" << i.val << ", %eax\n";
	}

	void AsmGenerator::acceptChar(ast::Char& c) {
		out << "\tmov $" << c.val << ", %al\n";
	}

	void AsmGenerator::acceptBlock(ast::Block& b) {
		for (auto& stmt : b.elems)
			stmt->visit(*this);
	}

	void AsmGenerator::acceptFnBody(ast::FnBody& f) {
		// Print function enter code
		out << "LFB0:\n"
			<< "\tpush %ebp\n"
			<< "\tmov %esp, %ebp\n"
			<< "\tpush %eax\n";

		// Print the body
		f.body->visit(*this);

		// Print function tail/endlog
		out << "\tleave\n"
			<< "\tret\n"
			<< "LFE0:\n";
	}

	void AsmGenerator::acceptVarAssign(ast::VarAssign& v) {
		// Print out function data
		out << "\t.p2align 4, 0x90\n"
			<< "\t.globl _main\n"								// This is rather hard to get at
			<< "\t.def _main; .scl 2; .type 32; .endef\n"
			<< "_main:\n";

		v.expr->visit(*this);

		// Print function ident information
		out << "\t.ident \"speroc: 0.0.1 (Windows 2016)\"";
	}
}