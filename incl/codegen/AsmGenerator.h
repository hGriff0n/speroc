#pragma once

#include "parser/visitor.h"

namespace spero::compiler::codegen {

	class AsmGenerator : public ast::Visitor {
		std::ostream& out;

		public:
			AsmGenerator(std::ostream&);

			virtual void accept(ast::Ast&) final;

			virtual void acceptBool(ast::Bool&) final;
			virtual void acceptByte(ast::Byte&) final;
			virtual void acceptFloat(ast::Float&) final;
			virtual void acceptInt(ast::Int&) final;
			virtual void acceptChar(ast::Char&) final;
			//virtual void acceptString(ast::String&) final;

			virtual void acceptBlock(ast::Block&) final;

			virtual void acceptFnBody(ast::FnBody&) final;
			virtual void acceptVarAssign(ast::VarAssign&) final;
	};

}