#pragma once

#include "ast.h"

namespace spero::compiler::ast {
	/*
	 * Forward Declarations
	 */
	struct Tuple;
	struct Array;
	struct TupleType;
	struct Pattern;
	struct PTuple;
	struct BasicBinding;


	/*
	 * Represents a globally applied annotation
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   name - binding that the annotation is associated to
	 *   args - arguments provided to the annotation
	 */
	struct GlobalAnnotation : Ast {
		ptr<BasicBinding> name;
		ptr<Tuple> args;

		GlobalAnnotation(ptr<BasicBinding>, ptr<Tuple>, Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "");
	};


	/*
	 * Represents an annotation instance
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   global - flag for whether the annotation applies to the compilation environment or context
	 *   name   - binding that the annotation is associated to
	 *   args   - arguments provided to the annotation
	 */
	struct Annotation : Ast {
		ptr<BasicBinding> name;
		ptr<Tuple> args;

		Annotation(ptr<BasicBinding>, ptr<Tuple>, Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Base class for pieces of generic declarations
	 *
	 * Extends: Ast
	 *   Reuses `prettyPrint` definition
	 *
	 * Exports:
	 *   name - name to bind the generic value to for the expression context
	 *   type - evaluable type of the binding
	 */
	struct GenericPart : Ast {
		ptr<BasicBinding> name;
		ptr<Type> type;

		GenericPart(ptr<BasicBinding>, ptr<Type>, Ast::Location);
	};


	/*
	 * Instance class for generics on typing information
	 *
	 * Extends: GenericPart
	 *
	 * Exports:
	 *   rel - required relationship of the instance field to a declared type
	 *   var - flag for the variance of the parent instance in relation to the instance field
	 */
	struct TypeGeneric : GenericPart {
		RelationType rel;
		VarianceType var;

		TypeGeneric(ptr<BasicBinding>, ptr<Type>, RelationType, VarianceType, Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Instance class for generics on value information
	 *
	 * Extends: GenericPart
	 *
	 * Exports:
	 *   value - default value for the generic field if none is provided or inferrable
	 */
	struct ValueGeneric : GenericPart {
		ptr<ValExpr> value;

		ValueGeneric(ptr<BasicBinding>, ptr<Type>, ptr<ValExpr>, Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Class that represents a single statement within a match construct
	 *
	 * Extends: ValExpr
	 *
	 * Exports:
	 *   vars - the pattern set to match against
	 *   expr - the expression to run on match success
	 */
	struct Case : ValExpr {
		ptr<PTuple> vars;
		ptr<ValExpr> expr;
		ptr<ValExpr> if_guard;

		Case(ptr<PTuple>, ptr<ValExpr>, ptr<ValExpr>, Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Base class for representing import pieces
	 *   Instance doubles for representing the any case 
	 *
	 * Extends: Ast
	 */
	struct ImportPiece : Ast {
		ImportPiece(Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "");
	};


	/*
	 * Instance class for representing named import pieces
	 *   Also handles rebinding if a second name is provided
	 *
	 * Extends: ImportPiece
	 *
	 * Exports:
	 *   name     - binding that will be imported into the current context
	 *   old_name - old binding that points to the implementation (TODO: Improve)
	 */
	struct ImportName : ImportPiece {
		ptr<BasicBinding> name;
		ptr<BasicBinding> new_name;
		ptr<Array> inst;

		ImportName(ptr<BasicBinding>, Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Represents a collection of import pieces (ala. '{...}')
	 *
	 * Extends: ImportPiece
	 *
	 * Exports:
	 *   imps - the collection of pieces to import
	 */
	struct ImportGroup : ImportPiece {
		std::deque<ptr<ImportPiece>> imps;

		ImportGroup(std::deque<ptr<ImportPiece>>, Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Represents an Algebraic Data Type constructor
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   name - the type name of the constructor
	 *   args - the collection of types that the constructor takes
	 */
	struct Adt : Ast {
		ptr<BasicBinding> name;
		ptr<TupleType> args;

		Adt(ptr<BasicBinding>, ptr<TupleType>, Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};


	/*
	 * Represents a value which will be provided in future analysis stages
	 *   mainly used for function forwarding to dot_control structures
	 *
	 * Extends: ValExpr
	 *
	 * Expors:
	 *   forwarded_from_fn - flag for whether the Future was created to represent a forwarded argument
	 */
	struct Future : ValExpr {
		bool forwarded_from_fn;

		Future(bool, Ast::Location);
		virtual OutStream& prettyPrint(OutStream&, size_t, std::string = "") final;
	};

}