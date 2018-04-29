#include "analysis/VarRefPass.h"

namespace spero::compiler::analysis {

	VarRefPass::VarRefPass(compiler::CompilationState& state, std::unique_ptr<SymTable> table) : globals{ std::move(table) }, state{ state } {
		current = globals.get();
	}

	std::unique_ptr<SymTable> VarRefPass::finalize() {
		return std::move(globals);
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
		//auto[nvar, iter] = lookup(*globals, current, *v.name);

		//// TODO: Handle looking for the correct definition based on the src location

		//if (iter != std::end(v.name->elems)) {
		//	state.log(ID::err, "Attempt to use undeclared variable `{}` <at {}>", v.name->elems.back()->name, v.loc);
		//	return;
		//}

		//if (!std::holds_alternative<VarData>(nvar->get())) {
		//	state.log(ID::err, "Attempt to use non-variable symbol `{}` as a variable <at {}>", v.name->elems.back()->name, v.loc);
		//}


		
		auto [nvar, iter] = _lookup(*globals, current, *v.name);

		if (iter != std::end(v.name->elems)) {
			if (testSsaLookupFailure(nvar, iter)) {
				state.log(ID::err, "Attempt to use variable `{}` before declaration in local scope context <at {}>", v.name->elems.back()->name, v.loc);
			} else {
				state.log(ID::err, "Attempt to use undeclared variable `{}` <at {}>", v.name->elems.back()->name, v.loc);
			}

			return;
		}

		if (!std::holds_alternative<ref_t<_VarData>>(*nvar)) {
			state.log(ID::err, "Attempt to use non-variable symbol `{}` as a variable <at {}>", v.name->elems.back()->name, v.loc);
		}
		
		\
	}

	void VarRefPass::visitBinOpCall(ast::BinOpCall& b) {
		AstVisitor::visitBinOpCall(b);

		if (b.op == "=") {
			auto* lhs = dynamic_cast<ast::Variable*>(b.lhs.get());
			if (!lhs) {
				state.log(ID::err, "Attempt to reassign a non-variable value <at {}>", lhs->loc);
				return;
			}

			// Perform all the necessary checks for reassignment
			auto[variable, iter] = _lookup(*globals, current, *lhs->name);

			// NOTE: Do we need to check whether the variable exists here ???
				// The error is already reported in an earlier stage so by this point, it is guaranteed to be valid
			/*if (iter == std::end(lhs->name->elems)) {
				if (auto* var = std::get_if<VarData>(&variable->get()); !var->is_mut) {
					state.log(ID::err, "Attempt to reassign immutable variable `{}` <at {}>", lhs->name->elems.back()->name, lhs->loc);
				}
			}*/
			
			if (iter == std::end(lhs->name->elems)) {
				if (auto* var = std::get_if<ref_t<_VarData>>(&*variable); !var->get().is_mut) {
					state.log(ID::err, "Attempt to reassign immutable variable `{}` <at {}>", lhs->name->elems.back()->name, lhs->loc);
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
