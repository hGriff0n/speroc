#include "codegen/AsmGenerator.h"
#include "parser/utils.h"

/*
 * Utility function to simplify emission of assembly instructions in several cases
 */
template<class T, class=std::enable_if_t<std::is_integral_v<T> && !std::is_convertible_v<T, std::string>>>
std::ostream& issueStore(std::ostream& s, const T& val, std::string loc) {
	return s << "\tmov $" << val << ", " << loc << '\n';
}
std::ostream& issueStore(std::ostream& s, std::string val, std::string loc) {
	return s << "\tmov " << val << ", " << loc << '\n';
}


namespace spero::compiler::gen {
	AsmGenerator::AsmGenerator(std::ostream& s) : out{ s }, curr_reg{ "eax" } {}


	// Base Nodes
	void AsmGenerator::accept(ast::Ast&) {}


	// Literals
	void AsmGenerator::acceptBool(ast::Bool& b) {
		if (b.val)
			issueStore(out, b.val, "%al");

		else
			out << "\txor %" << curr_reg << ", %" << curr_reg << '\n';
	}

	void AsmGenerator::acceptByte(ast::Byte& b) {
		out << "\txor %" << curr_reg << ", %" << curr_reg << "\n";
		issueStore(out, b.val, "%" + curr_reg);
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
		issueStore(out, i.val, "%" + curr_reg);
	}

	void AsmGenerator::acceptChar(ast::Char& c) {
		issueStore(out, c.val, "%al");
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

	void AsmGenerator::acceptBinOpCall(ast::BinOpCall& b) {
		// Evaluate the left hand and store it on the stack
		b.lhs->visit(*this);
		out << "\tpush %eax\n";
		
		// Evaluate the right hand
		b.rhs->visit(*this);


		if (b.op == "+") {
			out << "\tadd (%esp), %" << curr_reg << "\n\tadd $4, %esp\n";

		} else if (b.op == "-") {
			// Sub's a bit special because of the order of operations
			// This gets performed on the "on-stack" value, so I need to pop the stack to fix
			out << "\tsub %" << curr_reg << ", (%esp)\n\tpop %" << curr_reg << '\n';

		} else if (b.op == "*") {
			out << "\timul (%esp), %" << curr_reg << "\n\tadd $4, %esp\n";

		} else if (b.op == "/") {
			out << "\tcdq\n\tidiv (%esp), %" << curr_reg << "\n\tadd $4, %esp\n";
		}

	}

	void AsmGenerator::acceptUnaryOpApp(ast::UnaryOpApp& u) {
		// Evaluate the expression
		u.expr->visit(*this);

		switch (u.op) {
			case ast::UnaryType::MINUS:
				out << "\neg %" << curr_reg << '\n';
				break;
			case ast::UnaryType::NOT:
				out << "\ttest %" << curr_reg << ", %" << curr_reg << '\n';				// If the 'zero flag' isn't set yet
				out << "\tsetz %" << curr_reg << '\n';									// If the 'zero flag' is set
		}
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