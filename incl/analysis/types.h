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
	 * Base class for internal type representation
	 */
	class Type {
		// I don't think I can gain much by making this a flyweight
		const Type* cannonical_type = this;
		String name;

		bool is_mutable = false;
		bool is_view = false;
		bool is_reference = false;
		bool is_pointer = false;

		public:
			Type(String name) : name{ name } {}
			virtual ~Type() {}

			template<class S>
			friend S& operator<<(S&, const Type&);
	};

	template<class S>
	S& operator<<(S& stream, const Type& t) {
		return stream << t.name;
	}


	/*
	 * Fill the standard set of spero types for compiler recognition
	 *
	 * TODO: This is eventually going to be handled through automatic module loading
	 */
	inline AllTypes initTypeList() {
		static AllTypes types {
			{ "Int", std::make_shared<Type>("Int") },
			{ "Bool", std::make_shared<Type>("Bool") }
		};

		return types;
	}

}