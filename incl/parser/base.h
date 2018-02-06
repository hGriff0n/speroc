#pragma once

#include <memory>
#include "analysis/SymTable.h"

namespace spero::compiler {
	// Cut down on typing for unique_ptr
	template<class T> using ptr = std::unique_ptr<T>;
}

namespace spero::compiler::ast {
	/*
	 * Forward Declarations and Other types
	 */
	struct Visitor;

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

		Ast(Location);

		virtual void accept(Visitor&);
		virtual std::ostream& prettyPrint(std::ostream&, size_t, std::string = "");				// TODO: string_view
	};
}