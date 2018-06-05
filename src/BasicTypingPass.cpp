#include "analysis/BasicTypingPass.h"
#include "util/parser.h"

namespace spero::analysis {
	using namespace compiler;

	BasicTypingPass::BasicTypingPass(CompilationState& state, AllTypes& type_list, std::unique_ptr<SymTable> globals) : globals{ std::move(globals) }, state{ state }, type_list{ type_list } {
		current = globals.get();
	}

	std::unique_ptr<SymTable> BasicTypingPass::finalize() {
		return std::move(globals);
	}


	// Atoms
	void BasicTypingPass::visitInt(ast::Int& i) {
		if (type_list.count("Int") == 0) {
			state.log(ID::err, "Installation does not define an `Int` type <at {}>", i.loc);
		}

		i.type = type_list["Int"];
	}

	void BasicTypingPass::visitString(ast::String& i) {
		if (type_list.count("String") == 0) {
			state.log(ID::err, "Installation does not define a `String` type <at {}>", i.loc);
		}

		i.type = type_list["String"];
	}

	void BasicTypingPass::visitBool(ast::Bool& i) {
		if (type_list.count("Bool") == 0) {
			state.log(ID::err, "Installation does not define a `Bool` type <at {}>", i.loc);
		}

		i.type = type_list["Bool"];
	}

	// Decorations
	void BasicTypingPass::visitTypeAnnotation(ast::TypeAnnotation& t) {
		AstVisitor::visitTypeAnnotation(t);

		// TODO: The actual handling of type annotations is a lot more subtle than I've used here
			// But I need a better way of converting an 'ast::Type' into a 'analysis::Type'
		if (util::is_type<ast::SourceType>(t.typ)) {
			auto* type = util::view_as<ast::SourceType>(t.typ);

			if (type_list.count(type->name->elems.back()->name) == 0) {
				state.log(ID::err, "Installation does not define a `<>` type <at {}>", *type->name->elems.back(), t.loc);
			}

			// TODO: This should override the expression's type (if compatible), influence the expression's type (if unsettled), or throw an error (if incompatible)
			t.type = type_list[type->name->elems.back()->name];

		} else {
			t.type = t.expression->type;
		}

	}


	// Statements
	void BasicTypingPass::visitBinOpCall(ast::BinOpCall& b) {
		AstVisitor::visitBinOpCall(b);

		auto* lhs_type = b.lhs->type.get();
		auto* rhs_type = b.rhs->type.get();

		if (lhs_type != rhs_type && lhs_type && rhs_type) {
			state.log(ID::err, "Type: Operator `{}` is not defined for `{}, {}` <at {}>", b.op, *lhs_type, *rhs_type, b.loc);
		}
	}
}