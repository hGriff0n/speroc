#include "codegen/AsmGenerator.h"

namespace spero::compiler::codegen {
	AsmGenerator::AsmGenerator(std::ostream& s) : out{ s } {}

	void AsmGenerator::accept(ast::Ast&) {}

	void AsmGenerator::acceptBool(ast::Bool& b) {
		// movb	$1, %al
		// andb	$1, %al
		// movzbl	%al, %eax
		out << "\tmov $" << b.val << ", %eax\n";
	}

	void AsmGenerator::acceptByte(ast::Byte& b) {
		out << "\tmov $" << b.val << ", %eax\n";
	}

	void AsmGenerator::acceptFloat(ast::Float& f) {
		/*
			.def	 "?ref_float@@YAMXZ";
			.scl	2;
			.type	32;
			.endef
			.globl	__real@405ccccd
			.section	.rdata,"dr",discard,__real@405ccccd
			.p2align	2
		__real@405ccccd:
			.long	1079823565              # float 3.45000005
			.text
			.globl	"?ref_float@@YAMXZ"
			.p2align	4, 0x90
		"?ref_float@@YAMXZ":                    # @"\01?ref_float@@YAMXZ"
		# BB#0:
			pushl	%ebp
			movl	%esp, %ebp
			flds	__real@405ccccd
			popl	%ebp
			retl
		*/
	}

	void AsmGenerator::acceptInt(ast::Int& i) {
		out << "\tmov $" << i.val << ", %eax\n";
	}

	void AsmGenerator::acceptChar(ast::Char& c) {
		// movb	$97, %al
		// movsbl	%al, %eax
		out << "\tmov $" << c.val << ", %eax\n";
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