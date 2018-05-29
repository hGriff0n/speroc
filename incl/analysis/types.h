#pragma once

#include <deque>
#include <memory>
#include <unordered_map>

#include "spero_string.h"
//#include "analysis/SymTable.h"

// TODO: Need a way of associating type names to their 'Type*' instance

namespace spero::analysis {

	class Type;
	using AllTypes = std::unordered_map<String, std::shared_ptr<Type>>;

	/*
	 * Fill the standard set of spero types for compiler recognition
	 *
	 * TODO: This is eventually going to be handled through automatic module loading
	 */
	inline AllTypes initTypeList() {
		return {
			{ "Int", std::make_shared<Type>() },
			{ "Bool", std::make_shared<Type>() }
		};
	}

	/*
	 * Base class for internal type representation
	 */
	class Type {
		// I don't think I can gain much by making this a flyweight
		const Type* cannonical_type = this;

		bool is_mutable = false;
		bool is_view = false;
		bool is_reference = false;
		bool is_pointer = false;

		public:
			Type() {}
			virtual ~Type() {}
	};

}