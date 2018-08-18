#pragma once

#include "parser/AstVisitor.h"
#include "interface/CompilationState.h"
#include "util/analysis.h"
#include "analysis/AnalysisState.h"

namespace spero::analysis {

	/*
	 * Ast pass that collects all symbol declarations and introductions (where is it defined)
	 */
	class VarDeclPass : public compiler::ast::AstVisitor {
		compiler::CompilationState& state;
		analysis::AnalysisState& dictionary;

		// Since SymTables can contain references to other symtables, the addresses need to stay relatively set
		// For the moment, the global table is the only one that is not currently mapped to an ast node
		// To enforce invariant addressing, this is currently implemented as a std::unique_ptr
		// This may change once I have to integrate modules and types into this framework
		SymIndex current = GLOBAL_SYM_INDEX;

		// Track the current scoping context to be able to tailor analysis
		ScopingContext context = ScopingContext::GLOBAL;


		// TEMPORARY declarations
		compiler::ast::Interface* current_decl;
		int curr_ebp_offset = 0;

		public:
			VarDeclPass(compiler::CompilationState& state, AnalysisState& dict);

			// Decorations
			//virtual void visitAnnotation(compiler::ast::Annotation&) final;
			//virtual void visitAdt(compiler::ast::Adt&) final;
			virtual void visitArgument(compiler::ast::Argument&) final;
			//virtual void visitGenericPart(compiler::ast::GenericPart&) final;
			//virtual void visitTypeGeneric(compiler::ast::TypeGeneric&) final;
			//virtual void visitValueGeneric(compiler::ast::ValueGeneric&) final;

			// Types
			//virtual void visitSourceType(ast::SourceType&) final;

			// Atoms
			virtual void visitBlock(compiler::ast::Block&) final;
			virtual void visitFunction(compiler::ast::Function&) final;

			// Names
			virtual void visitAssignName(compiler::ast::AssignName&) final;
			//virtual void visitAssignTuple(compiler::ast::AssignTuple&) final;
			//virtual void visitTuplePattern(compiler::ast::TuplePattern&) final;
			//virtual void visitVarPattern(compiler::ast::VarPattern&) final;
			//virtual void visitAdtPattern(compiler::ast::AdtPattern&) final;

			// Control
			// NOTE: Solution to Case::expr lack of SymTable is probably generalizable to InAssign
			//virtual void visitCase(compiler::ast::Case&) final;

			// Statements
			virtual void visitInAssign(compiler::ast::InAssign&) final;
			//virtual void visitModDec(compiler::ast::ModDec&) final;
			//virtual void visitModRebindImport(compiler::ast::ModRebindImport&) final;
			//virtual void visitSingleImport(compiler::ast::SingleImport&) final;
			//virtual void visitMultipleImport(compiler::ast::MultipleImport&) final;
			//virtual void visitRebind(compiler::ast::Rebind&) final;
			virtual void visitInterface(compiler::ast::Interface&) final;
			//virtual void visitTypeAssign(compiler::ast::TypeAssign&) final;
			//virtual void visitVarAssign(compiler::ast::VarAssign&) final;
			//virtual void visitTypeExtension(compiler::ast::TypeExtension&) final;
	};

}
