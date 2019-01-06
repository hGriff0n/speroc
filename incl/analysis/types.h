#pragma once

#include <deque>
#include <memory>
#include <unordered_map>

#include "spero_string.h"
#include "analysis/SymTable.h"

namespace llvm {
	class Type;
}

namespace spero::analysis {

	class Type;
	using AllTypes = std::unordered_map<String, std::shared_ptr<Type>>;

	/*
	 * Base class for internal type representation
	 */
	class Type {
		const Type* cannonical_type = this;
		const SymTable* def = nullptr;			// This may have to be moved to a subtype (is there a type that doesn't need this - it doesn't have to use it)
		String name;

		// llvm translation helpers
		// TODO: Needed ???
		const llvm::Type* llvm_translation = nullptr;

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
	inline const AllTypes& getCoreTypeList() {
		static AllTypes types {
			{ "Int", std::make_shared<Type>("Int") },
			{ "Bool", std::make_shared<Type>("Bool") }
		};

		return types;
	}

}