#include "analysis/VarRefPass.h"

namespace spero::analysis {

	using namespace compiler;

	VarRefPass::VarRefPass(CompilationState& state, AnalysisState& dict) : dictionary{ dict }, state{ state } {}


	// Atoms
	void VarRefPass::visitBlock(ast::Block& b) {
		auto parent_scope = current;
		current = *b.locals;

		AstVisitor::visitBlock(b);

		current = parent_scope;
	}
	void VarRefPass::visitFunction(ast::Function& f) {
		auto parent_context = context;
		context = ScopingContext::SCOPE;

		AstVisitor::visitFunction(f);

		context = parent_context;
	}


	// Expressions
	void VarRefPass::visitVariable(ast::Variable& v) {
		// Perform some simple variable usage checks
		auto[def_table, iter] = lookup(dictionary.arena, current, *v.name);

		if (iter + 1 != std::end(v.name->elems)) {
			state.log(ID::err, "Attempt to use undeclared variable `{}` <at {}>", *v.name, v.loc);

			return;
		}

		auto& path_part = **iter;
		auto nvar = dictionary.arena[def_table].get(path_part.name, nullptr, path_part.loc, path_part.ssa_index);
		if (nvar) {
			if (!std::holds_alternative<ref_t<SymbolInfo>>(*nvar)) {
				state.log(ID::err, "Attempt to use non-variable symbol `{}` as a variable <at {}>", *v.name, v.loc);
			}

		// If the variable is bound, but 'get' gives nullopt, then declaration must happen after the use
		} else if (dictionary.arena[def_table].exists((**iter).name)) {
			state.log(ID::err, "attempt to use variable `{}` before declaration in local scope context <at {}>", *v.name, v.loc);
		} else {
			state.log(ID::err, "Attempt to use undeclared variable `{}` <at {}>", *v.name, v.loc);
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
			auto[def_table, iter] = lookup(dictionary.arena, current, *lhs->name);

			// NOTE: Do we need to check whether the variable exists here ???
				// The error is already reported in `visitVariable` so by this point, it is guaranteed to be valid
				// We do need to perform this check, because it is done in the same pass as `visitVariable` (we can't determine whether it succeeded)
			if (iter + 1 == std::end(lhs->name->elems)) {
				auto& pathPart = **iter;
				if (auto nvar = dictionary.arena[def_table].get(pathPart.name, nullptr, pathPart.loc, pathPart.ssa_index)) {
					if (auto* var = std::get_if<ref_t<SymbolInfo>>(&*nvar); !var->get().is_mut) {
						state.log(ID::err, "Attempt to reassign immutable variable `{}` <at {}>", *lhs->name, lhs->loc);
					}
				}
			}
		}
	}


	// Statements
	void VarRefPass::visitInAssign(ast::InAssign& in) {
		auto parent_scope = current;
		current = *in.binding;

		auto parent_context = context;
		context = ScopingContext::SCOPE;

		AstVisitor::visitInAssign(in);

		context = parent_context;
		current = parent_scope;
	}

	void VarRefPass::visitTypeAssign(ast::TypeAssign& t) {
		auto parent_context = context;
		context = ScopingContext::TYPE;

		AstVisitor::visitTypeAssign(t);

		context = parent_context;
	}


}
