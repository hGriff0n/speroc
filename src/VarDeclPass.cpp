#include "analysis/VarDeclPass.h"

#include "util/parser.h"

namespace spero::analysis {

	using namespace compiler;

	VarDeclPass::VarDeclPass(CompilationState& state, AnalysisState& dict) : dictionary{ dict }, state{ state } {}


	// Decorations
	void VarDeclPass::visitArgument(ast::Argument& arg) {
		// Create the VarData struct
		SymbolInfo info{ arg.loc, false };

		// Insert the new declaration into the table
		if (!dictionary.arena[current].insertArg(arg.name->name, info)) {
			state.log(ID::err, "Failed to insert argument symbol information: {} already existed <at {}>", arg.name->name, arg.loc);
		}
	}


	// Types


	// Atoms
	void VarDeclPass::visitBlock(ast::Block& b) {
		auto parent_scope = current;

		if (!b.locals.has_value()) {
			b.locals = dictionary.arena.size();
			current = *b.locals;

			// Create the new SymTable in the arena
			dictionary.arena.push_back(SymTable{ current, context });
			dictionary.arena.back().setParent(parent_scope);

		} else {
			current = *b.locals;
		}

		AstVisitor::visitBlock(b);

		current = parent_scope;

	}

	void VarDeclPass::visitFunction(ast::Function& f) {
		auto parent_scope = current;
		f.body->locals = dictionary.arena.size();

		auto parent_context = context;
		context = ScopingContext::SCOPE;

		// Create the SymTable in the arena
		dictionary.arena.push_back(SymTable{ *f.body->locals, context });
		dictionary.arena.back().setParent(parent_scope);

		AstVisitor::visitFunction(f);

		context = parent_context;
		current = parent_scope;
	}


	// Names
	void VarDeclPass::visitAssignName(ast::AssignName& n) {
		// Create the VarData struct
		SymbolInfo info{ n.loc, n.is_mut };
		if (auto* var = dynamic_cast<ast::VarAssign*>(current_decl)) {
			info.definition = var->expr.get();

			// Tell the function what it's name is (for analysis/assembly generation)
			if (auto* fn = dynamic_cast<ast::Function*>(var->expr.get())) {
				fn->name = n.var->name;
			}
			
		} else if (auto* type = dynamic_cast<ast::TypeAssign*>(current_decl)) {
			// TODO: This should actually create a sym table underneath
			info.definition = type->body.get();
		}
		
		// Insert the new declaration into the table
		if (!dictionary.arena[current].insert(n.var->name, info)) {
			state.log(ID::err, "Failed to insert symbol information: {} already bound to an import or SymTable <at {}>", n.var->name, n.loc);
		}
	}


	// Control


	// Statements
	void VarDeclPass::visitInAssign(ast::InAssign& in) {
		auto parent_scope = current;
		in.binding = dictionary.arena.size();
		current = *in.binding;

		auto parent_context = context;
		context = ScopingContext::SCOPE;

		// Create the new SymTable in the arena
		dictionary.arena.push_back(SymTable{ current, context });
		dictionary.arena.back().setParent(parent_scope);
		
		AstVisitor::visitInAssign(in);

		context = parent_context;
		current = parent_scope;
	}

	void VarDeclPass::visitInterface(ast::Interface& i) {
		ast::Interface* last = current_decl;
		current_decl = &i;

		AstVisitor::visitInterface(i);

		current_decl = last;
	}

	void VarDeclPass::visitTypeAssign(ast::TypeAssign& t) {
		auto parent_context = context;
		context = ScopingContext::TYPE;

		AstVisitor::visitTypeAssign(t);

		context = parent_context;
	}
}
