#include "codegen/AsmGenerator.h"
#include "util/parser.h"

namespace spero::compiler::gen {
	AsmGenerator::AsmGenerator(std::ostream& s, compiler::CompilationState& state) : out{ s }, emit { s }, state{ state } {}

	void AsmGenerator::loadVariable(ast::Variable& v) {
		auto var = v.name->toString();
		auto loc = current->getVar(var);

		if (!loc) {
			state.log(compiler::ID::err, "Attempt to use a variable before it was declared {}", compiler::CompilationState::location(v.loc));
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
	void AsmGenerator::visit(ast::Ast&) {}


	// Literals
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


	// Names
	void AsmGenerator::visitVariable(ast::Variable& v) {
		Register eax{ "eax" };
		out << "\tmov ";
		loadVariable(v);
		out << ", " << eax << '\n';
	}


	// Types


	// Decorations


	// Control
	void AsmGenerator::visitBlock(ast::Block& b) {
		// Reserve stack space for local variables
		//Register esp{ "esp" };
		//emit.add(4 * b.locals.getCount(), esp);
		
		// Initialize current
		//b.locals.setParent(current, true);
		//current = &b.locals;

		// Emit the code for the function body
		for (auto& stmt : b.elems) {
			stmt->accept(*this);
		}

		// Very basic stack cleanup code (just pop all the variables off the stack)
		emit.popByte(current->getCount());
		current = current->getParent();
	}


	// Statements
	void AsmGenerator::visitFunction(ast::Function& f) {
		Register ebp{ "ebp" }, esp{ "esp" }, eax{ "eax" };

		// Print function enter code
		emit.label("LFB0");
		emit.push(ebp);
		emit.mov(esp, ebp);
		emit.push(eax);

		// Print the body
		f.body->accept(*this);

		// Print function tail/endlog
		emit.leave();
		emit.ret();
		emit.label("LFE0");
	}

	void AsmGenerator::visitBinOpCall(ast::BinOpCall& b) {

	}

	//void AsmGenerator::visitReassign(ast::Reassign& r) {
	//	// Evaluate the operands
	//	r.val->visit(*this);
	//	performAssign(r.var->name->toString(), r.loc, false);
	//}

	void AsmGenerator::visitUnOpCall(ast::UnOpCall& u) {
		// Evaluate the expression
		u.expr->accept(*this);

		Register eax{ "eax" };

		// Perform the operator call
		// TODO: Perform type-based function lookup
		//switch (u.op) {
		//	case ast::UnaryType::MINUS:
		//		emit.neg(eax);
		//		break;
		//	case ast::UnaryType::NOT:
		//		if (!emit.zeroSet()) {				// Insert a 'test' instruction if the 'zero flag' isn't set
		//			emit.test(eax, eax);
		//		}
		//		emit.setz(eax);
		//}
	}

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
			performAssign(*v.name, true);			// Store the variable at its location
		}
	}
}
