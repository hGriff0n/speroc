#pragma once

#include "parser/AstVisitor.h"
#include "interface/CompilationState.h"
#include "util/analysis.h"

namespace spero::analysis {

	/*
	 * Ast pass that collects all symbol declarations and introductions (where is it defined)
	 *
	 * TODO: How will this pass handle 'after-use' declarations
	 *   If needed, maintain a vector of all "before-declaration" usages
	 *   Cross check that vector against all declarations before quitting the block/etc.
	 *   May need a bit of work to handle 'SSA' renaming (but all such cases should be the first instance, so just add '0' to the name)
	 */
	class VarDeclPass : public compiler::ast::AstVisitor {
		compiler::CompilationState& state;
		AllTypes& type_list;

		// Since SymTables can contain references to other symtables, the addresses need to stay relatively set
		// For the moment, the global table is the only one that is not currently mapped to an ast node
		// To enforce invariant addressing, this is currently implemented as a std::unique_ptr
		// This may change once I have to integrate modules and types into this framework
		std::unique_ptr<SymTable> globals;
		SymTable* current = nullptr;

		// Track the current scoping context to be able to tailor analysis
		ScopingContext context = ScopingContext::GLOBAL;

		public:
			VarDeclPass(compiler::CompilationState& state, AllTypes& type_list);
			std::unique_ptr<SymTable> finalize();

			// Decorations
			//virtual void visitAnnotation(ast::Annotation&) final;
			//virtual void visitAdt(ast::Adt&) final;
			//virtual void visitArgument(ast::Argument&) final;
			//virtual void visitGenericPart(ast::GenericPart&) final;
			//virtual void visitTypeGeneric(ast::TypeGeneric&) final;
			//virtual void visitValueGeneric(ast::ValueGeneric&) final;

			// Types
			//virtual void visitSourceType(ast::SourceType&) final;

			// Atoms
			virtual void visitBlock(compiler::ast::Block&) final;
			// NOTE: Assign the body's symbol table before visiting the arguments (should automatically scope them)
			//virtual void visitFunction(ast::Function&) final;

			// Names
			virtual void visitAssignName(compiler::ast::AssignName&) final;
			//virtual void visitAssignTuple(ast::AssignTuple&) final;
			//virtual void visitTuplePattern(ast::TuplePattern&) final;
			//virtual void visitVarPattern(ast::VarPattern&) final;
			//virtual void visitAdtPattern(ast::AdtPattern&) final;

			// Control
			// NOTE: Solution to Case::expr lack of SymTable is probably generalizable to InAssign
			//virtual void visitCase(ast::Case&) final;

			// Statements
			virtual void visitInAssign(compiler::ast::InAssign&) final;
			//virtual void visitModDec(ast::ModDec&) final;
			//virtual void visitModRebindImport(ast::ModRebindImport&) final;
			//virtual void visitSingleImport(ast::SingleImport&) final;
			//virtual void visitMultipleImport(ast::MultipleImport&) final;
			//virtual void visitRebind(ast::Rebind&) final;
			//virtual void visitInterface(ast::Interface&) final;
			//virtual void visitTypeAssign(ast::TypeAssign&) final;
			virtual void visitVarAssign(compiler::ast::VarAssign&) final;
			//virtual void visitTypeExtension(ast::TypeExtension&) final;
	};

}
