#include "analysis/VarDeclPass.h"

namespace spero::analysis {

	using namespace compiler;

	VarDeclPass::VarDeclPass(CompilationState& state) : globals{ std::make_unique<SymTable>() }, state{ state } {
		current = globals.get();
	}

	std::unique_ptr<SymTable> VarDeclPass::finalize() {
		return std::move(globals);
	}


	// Atoms
	void VarDeclPass::visitBlock(ast::Block& b) {
		auto* parent_scope = current;
		b.locals.setParent(current, true);
		current = &b.locals;

		AstVisitor::visitBlock(b);
		current = parent_scope;
	}


	// Names
	void VarDeclPass::visitAssignName(ast::AssignName& n) {
		// if (current == globals.get()) {

		// Register the variable in the current scope
		int off = -4 * (current->numVariables() + 1) - current->curr_ebp_offset;
		current->insert(n.var->name, VarData{ n.loc, n.is_mut, memory::Stack{ off } });
	}


	// Statements
	void VarDeclPass::visitInAssign(ast::InAssign& in) {
		auto* parent_scope = current;
		in.binding.setParent(current, true);
		current = &in.binding;

		AstVisitor::visitInAssign(in);
		current = parent_scope;
	}

	void VarDeclPass::visitVarAssign(ast::VarAssign& v) {
		// Because tuples can appear on the left hand side of the assignment
		// It's somewhat better for the 'mut' to be carried through the 'AssignPattern'
		v.name->is_mut = v.expr->is_mut;

		AstVisitor::visitVarAssign(v);
	}

}
