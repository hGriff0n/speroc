#pragma once

#include "parser/visitor.h"

namespace spero::compiler::gen {

	class AsmGenerator : public ast::Visitor {
		std::ostream& out;
		std::string curr_reg;

		public:
			AsmGenerator(std::ostream&);

			// Base Nodes
			virtual void accept(ast::Ast&) final;

			// Literals
			virtual void acceptBool(ast::Bool&) final;
			virtual void acceptByte(ast::Byte&) final;
			virtual void acceptFloat(ast::Float&) final;
			virtual void acceptInt(ast::Int&) final;
			virtual void acceptChar(ast::Char&) final;
			//virtual void acceptString(ast::String&) final;

			// Names

			// Types

			// Decorations

			// Control
			virtual void acceptBlock(ast::Block&) final;

			// Statements
			virtual void acceptFnBody(ast::FnBody&) final;
			virtual void acceptBinOpCall(ast::BinOpCall&) final;
			virtual void acceptVarAssign(ast::VarAssign&) final;
	};

}