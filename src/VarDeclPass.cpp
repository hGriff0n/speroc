#include "analysis/VarDeclPass.h"
#include "util/ranges.h"

namespace spero::compiler::analysis {

	VarDeclPass::VarDeclPass(compiler::CompilationState& state) : state{ state } {}

	SymTable VarDeclPass::finalize() {
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
		// Register the variable in the current scope
		if (!current->exists(n.var->name)) {
			int off = -4 * (current->numVariables() + 1) - current->ebp_offset;
			current->insert(n.var->name, analysis::VarData{ off, n.loc, n.is_mut });

		} else {
			// TODO: Change to SSA style renaming to accomodate shadowing
			state.log(ID::err, "Attempt to declare previously declared variable `{}` <at {}>", n.var->name, n.loc);
		}
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
		auto[nvar, iter] = lookup(globals, current, *v.name);

		if (iter != std::end(v.name->elems)) {
			state.log(ID::err, "Attempt to use undeclared variable `{}` <at {}>", v.name->elems.back()->name, v.loc);
			return;
		}

		if (!std::holds_alternative<analysis::VarData>(nvar->get())) {
			state.log(ID::err, "Attempt to use non-variable symbol `{}` as a variable <at {}>", v.name->elems.back()->name, v.loc);
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
		v.name->is_mut = v.expr->is_mut;

		AstVisitor::visitVarAssign(v);
	}

	void VarDeclPass::visitTypeExtension(ast::TypeExtension& t) {
		AstVisitor::visitTypeExtension(t);
	}



	using LookupType = std::optional<ref_t<analysis::DataType>>;
	std::tuple<std::optional<ref_t<analysis::DataType>>, ast::Path::iterator> lookup(analysis::SymTable& globals, analysis::SymTable* current, ast::Path& var_path) {
		auto[front, end] = util::range(var_path.elems);
		LookupType value = std::nullopt;
		bool has_next = true;

		// Support forced global indexing (through ':<>') (NOTE: Undecided on inclusion in final document)
		if (!(**front).name.empty()) {
			auto* next = current->mostRecentDef((**front).name);
			value = (*(next ? next : current))["self"];
			has_next = next != nullptr;

		} else {
			value = globals["self"];
			++front;
		}

		while (front != end && has_next) {
			auto next = std::visit([&](auto&& var) -> AccessType {
				if constexpr (std::is_same_v<std::decay_t<decltype(var)>, ref_t<analysis::SymTable>>) {
					return var.get().get((**front).name);
				}

				return AccessType{};
				}, value->get());

			if (has_next = next.has_value()) {
				value = next;
				++front;
			}
		}

		// Return the last accessed value and the last attempted symbol if lookup fails
		// This should simplify the process of assigning new variables, etc.
		return { value, front };
	}

}
