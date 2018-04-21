#include "codegen/AsmGenerator.h"
#include "util/parser.h"
#include "util/ranges.h"

using namespace asmjit;

namespace spero::compiler::gen {

	AsmGenerator::AsmGenerator(std::unique_ptr<analysis::SymTable> globals, compiler::CompilationState& state) : globals{ std::move(globals) }, state{ state } {
		current = this->globals.get();
		
		// TODO: Quick hack to enable 'batched' pushing of variables
		// Because the global scope isn't a table, the batch push didn't occur
		// NOTE: Will be fixed when I move to a static/global allocation scheme
		emit.pushWords(current->numVariables());
	}

	Assembler AsmGenerator::finalize() {
		emit.setAllocatedStack(globals->numVariables() * 4);
		return std::move(emit);
	}

	//
	// Literals
	//
	void AsmGenerator::visitBool(ast::Bool& b) {
		emit.mov(x86::eax, b.val);

		// NOTE: clang emits `xor rax, rax` for false
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
		// Initialize scope entry code
		auto* parent_scope = current;
		current = &b.locals;
		emit.pushWords(current->numVariables());
		// TODO: Zero initialize the allocated stack space?

		// Emit the code for the function body
		AstVisitor::visitBlock(b);

		// Very basic stack cleanup code (just pop all the variables off the stack)
		emit.popWords(current->numVariables());				// pop n bytes 
		current = parent_scope;
	}


	//
	// Names
	//
	void AsmGenerator::visitVariable(ast::Variable& v) {
		auto [nvar, _] = analysis::lookup(*globals, current, *v.name);
		(void)_;

		std::visit([&](auto&& var) {
			if constexpr (std::is_same_v<std::decay_t<decltype(var)>, analysis::VarData>) {
				emit.mov(x86::eax, x86::ptr(x86::ebp, var.off));
			}
		}, nvar->get());
	}

	void AsmGenerator::visitAssignName(ast::AssignName& n) {
		if (auto var = current->getVar(n.var->name); var) {
			emit.mov(x86::ptr(x86::ebp, var->get().off), x86::eax);
		}
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
			emit.write(".def _main");		// main is 64-bit entrypoint
			emit.writef(".scl %d", 2);
			emit.writef(".type %d", 32);
			emit.write(".endef");
			emit.write(".globl _main");
			emit.writef(".p2align %d, %x", 4, 0x90);
			//emit.write(".type _main, @function");

			emit.bind(emit.newNamedLabel("_main"));

			v.expr->accept(*this);

		} else {
			// TODO: Figure out a way to pass the mutability of the expression on to the 'name'
			v.expr->accept(*this);					// Push the expression value onto the stack
			v.name->is_mut = v.expr->is_mut;
			v.name->accept(*this);
		}
	}


	//
	// Expressions
	//
	void AsmGenerator::visitInAssign(ast::InAssign& in) {
		// Setup the symbol table to prevent the context from leaking
			// TODO: Could probably get a more efficient method by just adding a temporary key in the current table
			// TODO: Need to rewrite before 'variable pass' decoupling as this won't work in that framework
		auto* parent_scope = current;
		current = &in.binding;

		// Run through the assignment and expression
		in.bind->accept(*this);
		in.expr->accept(*this);

		// Pop off the symbol table
		emit.popWords(current->numVariables());
		current = parent_scope;
	}

	void AsmGenerator::visitFunction(ast::Function& f) {
		// Print function enter code
		// This is getting the 'main' label instead
		emit.bind(emit.newNamedLabel("LFB0"));
		emit.push(x86::ebp);
		emit.mov(x86::ebp, x86::esp);

		// Print the body
		f.body->accept(*this);

		// Print function tail/endlog
		emit.leave();
		emit.ret();
		emit.bind(emit.newNamedLabel("LFE0"));
	}

	void AsmGenerator::visitBinOpCall(ast::BinOpCall& b) {
		if (b.op == "=") {
			// Evaluate the value into the appropriate registers
			b.rhs->accept(*this);

			auto* lhs = dynamic_cast<ast::Variable*>(b.lhs.get());
			auto [variable, _] = analysis::lookup(*globals, current, *lhs->name);
			auto& var = std::get<analysis::VarData>(variable->get());

			emit.mov(x86::ptr(x86::ebp, var.off), x86::eax);
			(void)_;

		} else {
			// Evaluate the left side and store it on the stack
			b.lhs->accept(*this);
			emit.push(x86::eax);

			auto numPushedWords = 1;

			// Evaluate the right side
			b.rhs->accept(*this);

			// Perform the operator call
			if (b.op == "+") {
				emit.add(x86::eax, x86::ptr(x86::esp));

			} else if (b.op == "-") {
				emit.sub(x86::ptr(x86::esp), x86::eax);
				emit.mov(x86::eax, x86::ptr(x86::esp));

			} else if (b.op == "*") {
				emit.imul(x86::eax, x86::ptr(x86::esp));

			} else if (b.op == "/") {
				emit.xchg(x86::ptr(x86::esp), x86::eax);
				emit.pop(x86::ecx);
				emit.cdq();
				emit.idiv(x86::ecx);

				numPushedWords = 0;

			} else if (b.op == "==") {
				emit.cmp(x86::ptr(x86::esp), x86::eax);
				emit.mov(x86::eax, 0);
				emit.setz(x86::al);

			} else if (b.op == "!=") {
				emit.cmp(x86::ptr(x86::esp), x86::eax);
				emit.mov(x86::eax, 0);
				emit.setnz(x86::al);

			} else if (b.op == "<") {
				emit.cmp(x86::ptr(x86::esp), x86::eax);
				emit.mov(x86::eax, 0);
				emit.setc(x86::al);

			} else if (b.op == "<=") {
				emit.cmp(x86::eax, x86::ptr(x86::esp));
				emit.mov(x86::eax, 0);
				emit.setc(x86::al);
				emit.xor_(x86::eax, 1);

			} else if (b.op == ">") {
				emit.cmp(x86::eax, x86::ptr(x86::esp));
				emit.mov(x86::eax, 0);
				emit.setc(x86::al);

			} else if (b.op == ">=") {
				emit.cmp(x86::ptr(x86::esp), x86::eax);
				emit.mov(x86::eax, 0);
				emit.setc(x86::al);
				emit.xor_(x86::eax, 1);

			} else if (b.op == "%") {
				emit.xchg(x86::ptr(x86::esp), x86::eax);
				emit.pop(x86::ecx);
				emit.cdq();
				emit.idiv(x86::ecx);
				emit.mov(x86::eax, x86::edx);

				numPushedWords = 0;

			} else if (b.op == "&&") {
				emit.and_(x86::eax, x86::ptr(x86::esp));
				// TODO: Implement lazy evaluation (needed at this stage?)

			} else if (b.op == "||") {
				emit.or_(x86::eax, x86::ptr(x86::esp));
				// TODO: Implement lazy evaluation (needed at this stage?)

			}

			emit.popWords(numPushedWords);
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

namespace spero::compiler::ast {
	// NOTE: Even though this method is marked `abstract`, we have to give a definition because it's a constructor
	AstVisitor::~AstVisitor() {}
}
