#pragma once

#include "ast.h"

namespace spero::compiler::ast {
	/*
	 * Base class for all type nodes
	 *
	 * Extends: Ast
	 *   Reuses `prettyPrint` definition
	 *
	 * Exports:
	 *   id - type id, assigned during analysis
	 *   is_mut - flag whether values of the type are  mutable
	 *
	 * TODO:
	 *   Need to have a better handle on type id assignment and usage across the ast in the future
	 */
	struct Type : Ast {
		size_t id;
		bool is_mut = false;

		Type();
	};

	
	/*
	 * Basic instance class for type nodes
	 *
	 * Extends: Type
	 *
	 * Exports:
	 *   name - qualified binding that the type is refering to
	 *   pointer - any pointer/view styling applied to the type
	 */
	struct BasicType : Type {
		ptr<QualifiedBinding> name;
		PtrStyling pointer;
		
		BasicType(ptr<BasicBinding>, PtrStyling=PtrStyling::NA);
		BasicType(ptr<QualifiedBinding>, PtrStyling = PtrStyling::NA);
		virtual IoStream prettyPrint(IoStream, size_t = 0);
	};


	/*
	 * Basic instance class for generic instantiated types
	 *
	 * Extends: BasicType
	 *
	 * Exports:
	 *   inst - an array of generic parameters
	 */
	struct GenericType : Type {
		ptr<Array> inst;

		GenericType(ptr<QualifiedBinding>, ptr<Array>, PtrStyling);
		virtual IoStream prettyPrint(IoStream, size_t = 0);
	};

	
	/*
	 * Type instance class that represents a tuple of types
	 *
	 * Extends: Type
	 *
	 * Exports:
	 *   elems - the collection of types that construct the tuple
	 */
	struct TupleType : Type {
		std::deque<ptr<Type>> elems;

		TupleType(std::deque<ptr<Type>>&);
		virtual IoStream prettyPrint(IoStream, size_t = 0);
	};


	/*
	 * Type instance class that represents a function object
	 *
	 * Extends: Type

	 * Exports:
	 *   args - the tuple type that represents the functions arguments
	 *   ret - the type returned by the function
	 */
	struct FunctionType : Type {
		ptr<TupleType> args;
		ptr<Type> ret;

		FunctionType(ptr<TupleType>, ptr<Type>);
		virtual IoStream prettyPrint(IoStream, size_t = 0);
	};
}