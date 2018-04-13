#pragma once

#include "parser/AstVisitor.h"
#include "interface/CompilationState.h"

namespace spero::compiler::analysis {

	// If lookup succeeds, then the returned iterator is equal to `std::end(var_path.elems)`
	std::tuple<std::optional<ref_t<analysis::DataType>>, ast::Path::iterator> lookup(analysis::SymTable& globals, analysis::SymTable* current, ast::Path& var_path);


	/*
	 * Ast pass that collects and stores information for all variables
	 *
	 * TODO: How will this pass handle 'after-use' declarations
	 *   If needed, maintain a vector of all "before-declaration" usages
	 *   Cross check that vector against all declarations before quitting the block/etc.
	 *   May need a bit of work to handle 'SSA' renaming (but all such cases should be the first instance, so just add '0' to the name)
	 */
	class VarDeclPass : public ast::AstVisitor {
		compiler::CompilationState& state;

		analysis::SymTable globals;
		analysis::SymTable* current = &globals;

		public:
			VarDeclPass(compiler::CompilationState&);
			analysis::SymTable finalize();

			// Decorations
			virtual void visitAnnotation(ast::Annotation&) final;
			virtual void visitAdt(ast::Adt&) final;
			virtual void visitArgument(ast::Argument&) final;
			virtual void visitGenericPart(ast::GenericPart&) final;
			virtual void visitTypeGeneric(ast::TypeGeneric&) final;
			virtual void visitValueGeneric(ast::ValueGeneric&) final;

			// Types
			virtual void visitSourceType(ast::SourceType&) final;

			// Atoms
			virtual void visitBlock(ast::Block&) final;
			virtual void visitFunction(ast::Function&) final;

			// Names
			virtual void visitAssignName(ast::AssignName&) final;
			virtual void visitAssignTuple(ast::AssignTuple&) final;
			virtual void visitTuplePattern(ast::TuplePattern&) final;
			virtual void visitVarPattern(ast::VarPattern&) final;
			virtual void visitAdtPattern(ast::AdtPattern&) final;

			// Control
			// NOTE: Solution to Case::expr lack of SymTable is probably generalizable to InAssign
			virtual void visitCase(ast::Case&) final;

			// Expressions
			virtual void visitVariable(ast::Variable&) final;

			// Statements
			virtual void visitInAssign(ast::InAssign&) final;
			virtual void visitModDec(ast::ModDec&) final;
			virtual void visitModRebindImport(ast::ModRebindImport&) final;
			virtual void visitSingleImport(ast::SingleImport&) final;
			virtual void visitMultipleImport(ast::MultipleImport&) final;
			virtual void visitRebind(ast::Rebind&) final;
			virtual void visitInterface(ast::Interface&) final;
			virtual void visitTypeAssign(ast::TypeAssign&) final;
			virtual void visitVarAssign(ast::VarAssign&) final;
			virtual void visitTypeExtension(ast::TypeExtension&) final;
	};

}
