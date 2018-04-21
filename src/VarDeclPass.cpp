#include "analysis/VarDeclPass.h"

namespace spero::compiler::analysis {

	VarDeclPass::VarDeclPass(compiler::CompilationState& state) : globals{ std::make_unique<analysis::SymTable>() }, state{ state } {
		current = globals.get();
	}

	std::unique_ptr<analysis::SymTable> VarDeclPass::finalize() {
		return std::move(globals);
	}

	// Decorations
	void VarDeclPass::visitAnnotation(ast::Annotation& a) {
		AstVisitor::visitAnnotation(a);
	}

	void VarDeclPass::visitAdt(ast::Adt& a) {
		AstVisitor::visitAdt(a);
	}

	void VarDeclPass::visitArgument(ast::Argument& a) {
		AstVisitor::visitArgument(a);
	}

	void VarDeclPass::visitGenericPart(ast::GenericPart& g) {
		AstVisitor::visitGenericPart(g);
	}

	void VarDeclPass::visitTypeGeneric(ast::TypeGeneric& t) {
		AstVisitor::visitTypeGeneric(t);
	}

	void VarDeclPass::visitValueGeneric(ast::ValueGeneric& v) {
		AstVisitor::visitValueGeneric(v);
	}


	// Types
	void VarDeclPass::visitSourceType(ast::SourceType& s) {
		AstVisitor::visitSourceType(s);
	}


	// Atoms
	void VarDeclPass::visitBlock(ast::Block& b) {
		auto* parent_scope = current;
		b.locals.setParent(current, true);
		current = &b.locals;

		AstVisitor::visitBlock(b);
		current = parent_scope;
	}

	void VarDeclPass::visitFunction(ast::Function& f) {
		AstVisitor::visitFunction(f);
	}


	// Names
	void VarDeclPass::visitAssignName(ast::AssignName& n) {
		// Perform 'SSA' style renaming to support variable shadowing
		n.var->name = current->allocateName(n.var->name);

		// Register the variable in the current scope
		int off = -4 * (current->numVariables() + 1) - current->ebp_offset;
		current->insert(n.var->name, analysis::VarData{ n.loc, off, current == globals.get(), n.is_mut });
	}

	void VarDeclPass::visitAssignTuple(ast::AssignTuple& a) {
		AstVisitor::visitAssignTuple(a);
	}

	void VarDeclPass::visitTuplePattern(ast::TuplePattern& t) {
		AstVisitor::visitTuplePattern(t);
	}

	void VarDeclPass::visitVarPattern(ast::VarPattern& v) {
		AstVisitor::visitVarPattern(v);
	}

	void VarDeclPass::visitAdtPattern(ast::AdtPattern& a) {
		AstVisitor::visitAdtPattern(a);
	}


	// Control
	void VarDeclPass::visitCase(ast::Case& c) {
		AstVisitor::visitCase(c);
	}


	// Expressions
	void VarDeclPass::visitVariable(ast::Variable& v) {
		// Perform some simple variable usage checks
		auto[nvar, iter] = lookup(*globals, current, *v.name);

		if (iter != std::end(v.name->elems)) {
			state.log(ID::err, "Attempt to use undeclared variable `{}` <at {}>", v.name->elems.back()->name, v.loc);
			return;
		}

		if (!std::holds_alternative<analysis::VarData>(nvar->get())) {
			state.log(ID::err, "Attempt to use non-variable symbol `{}` as a variable <at {}>", v.name->elems.back()->name, v.loc);
		}
	}

	void VarDeclPass::visitBinOpCall(ast::BinOpCall& b) {
		AstVisitor::visitBinOpCall(b);

		if (b.op == "=") {
			auto* lhs = dynamic_cast<ast::Variable*>(b.lhs.get());
			if (!lhs) {
				state.log(ID::err, "Attempt to reassign a non-variable value <at {}>", lhs->loc);
				return;
			}

			// Perform all the necessary checks for reassignment
			auto [variable, iter] = analysis::lookup(*globals, current, *lhs->name);

			// I think some of these will be handled
			if (iter != std::end(lhs->name->elems)) {
				state.log(ID::err, "Attempt to reassign undeclared variable `{}` <at {}>", lhs->name->elems.back()->name, lhs->loc);

			} else if (!std::holds_alternative<analysis::VarData>(variable->get())) {
				state.log(ID::err, "Attempt to reassign non-variable symbol `{}` <at {}>", lhs->name->elems.back()->name, lhs->loc);

			} else if (auto& var = std::get<analysis::VarData>(variable->get()); !var.is_mut) {
				state.log(ID::err, "Attempt to reassign immutable variable `{}` <at {}>", lhs->name->elems.back()->name, lhs->loc);
			}
		}
	}


	// Statements
	void VarDeclPass::visitInAssign(ast::InAssign& in) {
		auto* parent_scope = current;
		in.binding.setParent(current, true);
		current = &in.binding;

		AstVisitor::visitInAssign(in);
		current = parent_scope;
	}

	void VarDeclPass::visitModDec(ast::ModDec& m) {
		AstVisitor::visitModDec(m);
	}

	void VarDeclPass::visitModRebindImport(ast::ModRebindImport& m) {
		AstVisitor::visitModRebindImport(m);
	}

	void VarDeclPass::visitSingleImport(ast::SingleImport& s) {
		AstVisitor::visitSingleImport(s);
	}

	void VarDeclPass::visitMultipleImport(ast::MultipleImport& m) {
		AstVisitor::visitMultipleImport(m);
	}

	void VarDeclPass::visitRebind(ast::Rebind& r) {
		AstVisitor::visitRebind(r);
	}

	void VarDeclPass::visitInterface(ast::Interface& i) {
		AstVisitor::visitInterface(i);
	}

	void VarDeclPass::visitTypeAssign(ast::TypeAssign& t) {
		AstVisitor::visitTypeAssign(t);
	}

	void VarDeclPass::visitVarAssign(ast::VarAssign& v) {
		// Because tuples can appear on the left hand side of the assignment
		// It's somewhat better for the 'mut' to be carried through the 'AssignPattern'
		v.name->is_mut = v.expr->is_mut;

		AstVisitor::visitVarAssign(v);
	}

	void VarDeclPass::visitTypeExtension(ast::TypeExtension& t) {
		AstVisitor::visitTypeExtension(t);
	}

}
