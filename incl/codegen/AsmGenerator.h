#pragma once

#include "parser/visitor.h"

namespace spero::compiler::codegen {

	class AsmGenerator : public ast::Visitor {
		std::ostream& out;

		public:
			AsmGenerator(std::ostream&);

			virtual void accept(ast::Ast&) final;

			virtual void acceptVarAssign(ast::VarAssign&) final;
			virtual void acceptInt(ast::Int&) final;
			virtual void acceptFnBody(ast::FnBody&) final;
	};

}