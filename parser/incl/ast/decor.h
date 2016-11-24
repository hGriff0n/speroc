#pragma once

#include "ast.h"
#include <optional>

namespace spero::compiler::ast {
	/*
	 * Forward Declarations
	 */
	struct BasicBinding;
	struct Tuple;
	struct Type;
	struct TupleType;
	struct Pattern;


	// Initial look at splitting global annotations into a separate ast class
	struct GlobalAnnotation : Ast {};
	struct LocalAnnotation : GlobalAnnotation {};

	/*
	 * Represents an annotation instance
	 *
	 * Extends: Ast
	 *
	 * Exports:
	 *   global - flag for whether the annotation applies to the compilation environment or context
	 *   name   - binding that the annotation is associated to
	 *   args   - arguments provided to the annotation
	 *
	 * TODO: Split into global and local annotation classes (Global annotations are statements themselves)
	 */
	struct Annotation : Ast {
		bool global;
		ptr<BasicBinding> name;
		std::optional<ptr<Tuple>> args;

		Annotation(ptr<BasicBinding>, ptr<Tuple>, bool);
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

		GenericPart(ptr<BasicBinding>, ptr<Type>);
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

		TypeGeneric(ptr<BasicBinding>, ptr<Type>, RelationType, VarianceType);
		virtual OutStream prettyPrint(OutStream, size_t = 0);
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
		std::optional<ptr<ValExpr>> value;

		ValueGeneric(ptr<BasicBinding>, ptr<Type>, ptr<ValExpr>);
		virtual OutStream prettyPrint(OutStream, size_t = 0);
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
		std::deque<ptr<Pattern>> vars;
		ptr<ValExpr> expr;

		Case(ptr<ValExpr>);
		virtual OutStream prettyPrint(OutStream, size_t = 0);
	};


	/*
	 * Base class for representing import pieces
	 *   Instance doubles for representing the any case 
	 *
	 * Extends: Ast
	 */
	struct ImportPiece : Ast {
		virtual OutStream prettyPrint(OutStream, size_t = 0);
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
		std::optional<ptr<BasicBinding>> old_name;

		ImportName(ptr<BasicBinding>);
		ImportName(ptr<BasicBinding>, ptr<BasicBinding>);
		virtual OutStream prettyPrint(OutStream, size_t = 0);
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

		ImportGroup(std::deque<ptr<ImportPiece>>&);
		virtual OutStream prettyPrint(OutStream, size_t = 0);
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

		Adt(ptr<BasicBinding>, ptr<TupleType>);
		virtual OutStream prettyPrint(OutStream, size_t = 0);
	};
}