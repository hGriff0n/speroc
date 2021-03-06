#pragma once

#include <memory>

#include "parser/location.h"

namespace spero::compiler {
	// Cut down on typing for unique_ptr
	template<class T>
	using ptr = std::unique_ptr<T>;
}

namespace spero::compiler::ast {
	/*
	 * Forward Declarations and Other types
	 */
	struct AstVisitor;

	/*
	 * Base class for all ast nodes
	 *
	 * Specifies the required information that all ast nodes must contain
	 *
	 * Exports:
	 *   loc - Structure containing the location data for the node
	 *   visit - Polymorphic method to accept a visitor object for iteration
	 */
	struct Ast {
		Location loc;

		Ast(Location loc);

		virtual void accept(AstVisitor& v);
		virtual std::ostream& prettyPrint(std::ostream& s, size_t buf, std::string_view context = "");
	};
}