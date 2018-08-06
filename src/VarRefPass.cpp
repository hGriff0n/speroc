#include "analysis/VarRefPass.h"

namespace spero::analysis {

	using namespace compiler;

	VarRefPass::VarRefPass(CompilationState& state, AnalysisState& dict) : dictionary{ dict }, state{ state } {
		current = dictionary.table.get();
	}


	// Atoms
	void VarRefPass::visitBlock(ast::Block& b) {
		auto* parent_scope = current;
		current = &b.locals;
		AstVisitor::visitBlock(b);
		current = parent_scope;
	}


	// Expressions
	void VarRefPass::visitVariable(ast::Variable& v) {
		// Perform some simple variable usage checks
		auto [nvar, iter] = lookup(*dictionary.table, current, *v.name);

		if (iter != std::end(v.name->elems)) {
			if (testSsaLookupFailure(nvar, iter)) {
				state.log(ID::err, "Attempt to use variable `{}` before declaration in local scope context <at {}>", *v.name, v.loc);
			} else {
				state.log(ID::err, "Attempt to use undeclared variable `{}` <at {}>", *v.name, v.loc);
			}

			return;
		}

		if (!std::holds_alternative<ref_t<VarData>>(*nvar)) {
			state.log(ID::err, "Attempt to use non-variable symbol `{}` as a variable <at {}>", *v.name, v.loc);
		}
	}

	void VarRefPass::visitBinOpCall(ast::BinOpCall& b) {
		AstVisitor::visitBinOpCall(b);

		if (b.op == "=") {
			auto* lhs = dynamic_cast<ast::Variable*>(b.lhs.get());
			if (!lhs) {
				state.log(ID::err, "Attempt to reassign a non-variable value <at {}>", lhs->loc);
				return;
			}

			auto& most_qualified_part = lhs->name->elems.back()->name;
			if (most_qualified_part == "self" || most_qualified_part == "super") {
				state.log(ID::err, "Attempt to reassign `{}` keyword <at {}>", most_qualified_part, lhs->loc);
				return;
			}

			// Perform all the necessary checks for reassignment
			auto[variable, iter] = lookup(*dictionary.table, current, *lhs->name);

			// NOTE: Do we need to check whether the variable exists here ???
				// The error is already reported in `visitVariable` so by this point, it is guaranteed to be valid
				// We do need to perform this check, because it is done in the same pass as `visitVariable` (we can't determine whether it succeeded)
			if (iter == std::end(lhs->name->elems)) {
				if (auto* var = std::get_if<ref_t<VarData>>(&*variable); !var->get().is_mut) {
					state.log(ID::err, "Attempt to reassign immutable variable `{}` <at {}>", *lhs->name, lhs->loc);
				}
			}
		}
	}


	// Statements
	void VarRefPass::visitInAssign(ast::InAssign& in) {
		auto* parent_scope = current;
		current = &in.binding;

		AstVisitor::visitInAssign(in);

		current = parent_scope;
	}

}