#pragma once

#include "ast.h"

namespace spero::compiler::ast {

#define DEF_VISIT(typ) inline virtual void visit##typ(typ##& t)
#define abstract =0

	/*
	 * AstVisitor class definition
	 *
	 * Exports:
	 *   visit* - accept method that is called within the visit function to invoke
	 */
	struct AstVisitor {
		virtual ~AstVisitor() {}

		// Base Nodes
		DEF_VISIT(Ast) {}
		DEF_VISIT(Token) {}
		DEF_VISIT(Statement) {
			for (auto&& anot : t.annots) {
				anot->accept(*this);
			}
		}
		DEF_VISIT(ValExpr) {
			visitStatement(t);

			/*if (t.type) {
				t.type->accept(*this);
			}*/
		}

		// Literals
		DEF_VISIT(Literal) {
			visitValExpr(t);
		}
		DEF_VISIT(Bool) {
			visitLiteral(t);
		}
		DEF_VISIT(Byte) {
			visitLiteral(t);
		}
		DEF_VISIT(Float) {
			visitLiteral(t);
		}
		DEF_VISIT(Int) {
			visitLiteral(t);
		}
		DEF_VISIT(Char) {
			visitLiteral(t);
		}
		DEF_VISIT(String) {
			visitValExpr(t);
		}
		DEF_VISIT(Future) {
			visitValExpr(t);
		}

		// Atoms
		DEF_VISIT(Tuple) {
			visitValExpr(t);

			for (auto&& elem : t.elems) {
				elem->accept(*this);
			}
		}
		DEF_VISIT(Array) {
			visitValExpr(t);

			for (auto&& elem : t.elems) {
				elem->accept(*this);
			}
		}
		DEF_VISIT(Block) {
			visitValExpr(t);

			for (auto&& elem : t.elems) {
				elem->accept(*this);
			}
		}
		DEF_VISIT(Function) {
			visitValExpr(t);

			if (t.args.size()) {
				for (auto&& arg : t.args) {
					arg->accept(*this);
				}
			}

			t.body->accept(*this);
		}

		// Names
		DEF_VISIT(BasicBinding) {}
		DEF_VISIT(PathPart) {
			if (t.gens) {
				t.gens->accept(*this);
			}
		}
		DEF_VISIT(Path) {
			for (auto&& part : t.elems) {
				part->accept(*this);
			}
		}
		DEF_VISIT(Pattern) {}
		DEF_VISIT(TuplePattern) {
			visitPattern(t);

			for (auto& elem : t.elems) {
				elem->accept(*this);
			}
		}
		DEF_VISIT(VarPattern) {
			visitPattern(t);

			t.name->accept(*this);
		}
		DEF_VISIT(AdtPattern) {
			visitVarPattern(t);

			if (t.args) {
				t.args->accept(*this);
			}
		}
		DEF_VISIT(ValPattern) {
			visitPattern(t);

			t.val->accept(*this);
		}
		DEF_VISIT(AssignPattern) {}
		DEF_VISIT(AssignName) {
			visitAssignPattern(t);

			t.var->accept(*this);
		}
		DEF_VISIT(AssignTuple) {
			visitAssignPattern(t);

			for (auto&& elem : t.elems) {
				elem->accept(*this);
			}
		}

		// Types
		DEF_VISIT(Type) {}
		DEF_VISIT(SourceType) {
			visitType(t);

			t.name->accept(*this);
		}
		DEF_VISIT(GenericType) {
			visitSourceType(t);

			if (t.inst) {
				t.inst->accept(*this);
			}
		}
		DEF_VISIT(TupleType) {
			visitType(t);

			for (auto&& elem : t.elems) {
				elem->accept(*this);
			}
		}
		DEF_VISIT(FunctionType) {
			visitType(t);

			t.args->accept(*this);

			// NOTE: It is an error if `t.ret == nullptr`
			if (t.ret) {
				t.ret->accept(*this);
			}
		}
		DEF_VISIT(AndType) {
			visitType(t);

			for (auto&& elem : t.elems) {
				// NOTE: It is an error if `elem == nullptr`
				if (elem) {
					elem->accept(*this);
				}
			}
		}
		DEF_VISIT(OrType) {
			visitType(t);

			for (auto& elem : t.elems) {
				elem->accept(*this);
			}
		}

		// Decorations
		DEF_VISIT(Annotation) {
			t.name->accept(*this);
			if (t.args) {
				t.args->accept(*this);
			}
		}
		DEF_VISIT(LocalAnnotation) {
			visitAnnotation(t);
		}
		DEF_VISIT(GenericPart) {
			if (t.type) {
				t.type->accept(*this);
			}
			t.name->accept(*this);
		}
		DEF_VISIT(TypeGeneric) {
			visitGenericPart(t);

			if (t.default_type) {
				t.default_type->accept(*this);
			}
		}
		DEF_VISIT(ValueGeneric) {
			visitGenericPart(t);

			if (t.default_val) {
				t.default_val->accept(*this);
			}
		}
		DEF_VISIT(LitGeneric) {
			visitGenericPart(t);

			t.value->accept(*this);
		}
		DEF_VISIT(GenericArray) {
			for (auto&& elem : t.elems) {
				elem->accept(*this);
			}
		}
		DEF_VISIT(Constructor) {}
		DEF_VISIT(Adt) {
			visitConstructor(t);

			t.name->accept(*this);
			if (t.args) {
				t.args->accept(*this);
			}
		}
		DEF_VISIT(TypeAnnotation) {
			visitValExpr(t);

			t.typ->accept(*this);
			t.expression->accept(*this);
		}
		DEF_VISIT(Argument) {
			t.name->accept(*this);
			if (t.typ) {
				t.typ->accept(*this);
			}
		}
		DEF_VISIT(ArgTuple) {
			visitConstructor(t);

			for (auto&& elem : t.elems) {
				elem->accept(*this);
			}
		}

		// Control
		DEF_VISIT(Branch) {
			visitValExpr(t);
		}
		DEF_VISIT(Loop) {
			visitBranch(t);

			t.body->accept(*this);
		}
		DEF_VISIT(While) {
			visitBranch(t);

			t.test->accept(*this);
			t.body->accept(*this);
		}
		DEF_VISIT(For) {
			visitBranch(t);

			t.pattern->accept(*this);
			t.generator->accept(*this);
			t.body->accept(*this);
		}
		DEF_VISIT(IfBranch) {
			visitBranch(t);

			t.test->accept(*this);
			t.body->accept(*this);
		}
		DEF_VISIT(IfElse) {
			visitBranch(t);

			for (auto&& elem : t.elems) {
				elem->accept(*this);
			}

			if (t.else_) {
				t.else_->accept(*this);
			}
		}
		DEF_VISIT(Case) {
			visitValExpr(t);

			t.vars->accept(*this);
			if (t.if_guard) {
				t.if_guard->accept(*this);
			}
			t.expr->accept(*this);
		}
		DEF_VISIT(Match) {
			visitBranch(t);

			t.switch_expr->accept(*this);
			for (auto&& _case_ : t.cases) {
				_case_->accept(*this);
			}
		}
		DEF_VISIT(Jump) {
			visitBranch(t);

			if (t.expr) {
				t.expr->accept(*this);
			}
		}

		// Statements
		DEF_VISIT(ModDec) {
			visitStatement(t);

			t._module->accept(*this);
		}
		DEF_VISIT(ImplExpr) {
			visitStatement(t);

			t._interface->accept(*this);
			if (t.type) {
				t.type->accept(*this);
			}
			if (t.impls) {
				t.impls->accept(*this);
			}
		}
		DEF_VISIT(ModRebindImport) {
			visitStatement(t);

			if (t._module) {
				t._module->accept(*this);
			} else {
				// TODO: This rebinds into/from the current scope
			}
		}
		DEF_VISIT(SingleImport) {
			visitModRebindImport(t);
		}
		DEF_VISIT(MultipleImport) {
			visitModRebindImport(t);

			for (auto&& elem : t.elems) {
				elem->accept(*this);
			}
		}
		DEF_VISIT(Rebind) {
			visitModRebindImport(t);

			t.new_name->accept(*this);
		}
		DEF_VISIT(Interface) {
			visitStatement(t);

			t.name->accept(*this);
			if (t.gen) {
				t.gen->accept(*this);
			}
			if (t.type) {
				t.type->accept(*this);
			}
		}
		DEF_VISIT(TypeAssign) {
			visitInterface(t);

			for (auto&& constructor : t.cons) {
				constructor->accept(*this);
			}

			t.body->accept(*this);
		}
		DEF_VISIT(VarAssign) {
			t.expr->accept(*this);

			visitInterface(t);
		}
		DEF_VISIT(TypeExtension) {
			visitValExpr(t);

			t.typ_name->accept(*this);
			if (t.args) {
				t.args->accept(*this);
			}
			t.ext->accept(*this);
		}

		// Expression
		DEF_VISIT(InAssign) {
			visitValExpr(t);

			t.bind->accept(*this);
			t.expr->accept(*this);
		}
		DEF_VISIT(Variable) {
			visitValExpr(t);

			t.name->accept(*this);
		}
		DEF_VISIT(UnOpCall) {
			visitValExpr(t);

			t.op->accept(*this);
			t.expr->accept(*this);
		}
		DEF_VISIT(BinOpCall) {
			visitValExpr(t);

			t.lhs->accept(*this);
			t.rhs->accept(*this);
		}
		DEF_VISIT(Index) {
			visitValExpr(t);

			for (auto&& elem : t.elems) {
				elem->accept(*this);
			}
		}
		DEF_VISIT(FnCall) {
			visitValExpr(t);

			if (t.arguments) {
				t.arguments->accept(*this);
			}
			t.callee->accept(*this);
		}

		// Errors
		//DEF_VISITOR(Error);
	};

	template <class T>
	void visit(AstVisitor& visitor, T&& ast_stack) {
		for (auto&& node : ast_stack) {
			node->accept(visitor);
		}
	}

#undef DEF_VISIT
#undef abstract

}