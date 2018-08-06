#include "analysis/VarDeclPass.h"

#include "util/parser.h"

namespace spero::analysis {

	using namespace compiler;

	VarDeclPass::VarDeclPass(CompilationState& state, AnalysisState& dict) : dictionary{ dict }, state{ state } {
		current = dictionary.table.get();
	}


	// Decorations
	void VarDeclPass::visitArgument(ast::Argument& arg) {
		// TODO: Copy-pasted from visitAssignName. Doesn't handle the vagaries of arguments
		int off = -4 * (current->numVariables() + 1) - current->curr_ebp_offset;

		// Create the VarData struct
		VarData info{ arg.loc, false, memory::Stack{ off } };

		// Insert the new declaration into the table
		current->insert(arg.name->name, info);
	}


	// Types


	// Atoms
	void VarDeclPass::visitBlock(ast::Block& b) {
		auto* parent_scope = current;
		b.locals.setParent(current, true);
		current = &b.locals;

		AstVisitor::visitBlock(b);
		current = parent_scope;
	}

	void VarDeclPass::visitFunction(ast::Function& f) {
		auto parent_scope = current;
		f.body->locals.setParent(current, true);
		current = &f.body->locals;

		// TODO: Figure out how to get arguments declared in the symtable
		for (auto&& arg : f.args) {
			arg->accept(*this);
		}
		current->setNumArgs(f.args.size());
		
		current = parent_scope;

		AstVisitor::visitFunction(f);
	}


	// Names
	void VarDeclPass::visitAssignName(ast::AssignName& n) {
		// TODO: Move this to after the typing stage (for monomorphism, etc.)
		if (n.var->name == "main") {
			n.var->name = "_main";
		}

		// Register the variable in the current scope
		int off = -4 * (current->numVariables() + 1) - current->curr_ebp_offset;

		// Create the VarData struct
		VarData info{ n.loc, n.is_mut, memory::Stack{ off } };
		if (auto* var = dynamic_cast<ast::VarAssign*>(current_decl)) {
			info.definition = var->expr.get();

			// Tell the function what it's name is (for analysis/assembly generation)
			if (auto* fn = dynamic_cast<ast::Function*>(info.definition)) {
				fn->name = n.var->name;
				info.storage = analysis::memory::Global{ 0, false };
			}
			
		} else if (auto* type = dynamic_cast<ast::TypeAssign*>(current_decl)) {
			// TODO: This should actually create a sym table underneath
			info.definition = type->body.get();
		}
		
		// Insert the new declaration into the table
		current->insert(n.var->name, info);
	}


	// Control


	// Statements
	void VarDeclPass::visitInAssign(ast::InAssign& in) {
		auto* parent_scope = current;
		in.binding.setParent(current, true);
		current = &in.binding;

		AstVisitor::visitInAssign(in);
		current = parent_scope;
	}

	void VarDeclPass::visitInterface(ast::Interface& i) {
		ast::Interface* last = current_decl;
		current_decl = &i;

		AstVisitor::visitInterface(i);

		current_decl = last;
	}
}