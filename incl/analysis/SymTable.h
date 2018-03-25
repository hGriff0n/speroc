#pragma once

#include <unordered_map>
#include <optional>
#include <variant>

#include "parser/location.h"

template<class T>
using ref_t = std::reference_wrapper<T>;

namespace spero::compiler::analysis {
	struct VarData;
	struct OverloadSet;
	class SymTable;

	using DataType = std::variant<VarData, OverloadSet, SymTable>;


	// Tables will implicitly support single step indexing to get a symbol table or variable data structure
		// TODO: Come up with a quick way to insert/get a qualified variable (note: these are probably free functions/lambdas)
			// get should always fail if any step on the way doesn't exist
			// insertion should probably fail as well
				// the only time where this behavior wouldn't make sense is in nested modules
				// however, i can special case that or handle it manually
			// However this isn't the domain of the SymTable, I just need to handle one step here
	// TODO: How should importing a module be handled in regards to all of its information (leave until I implement modules)
		// Imports are going to take the form of explicitly assigning a reference/pointer of the imported object to the imported name
			// "use std:io" will create an entry directing towards "std:io" under the name "io" in the current scope
		// TODO: How to fucking type and interface that ??
			// You also have to make using an imported variable indistinct from using a declared variable
	// NOTE: Individual functions will be inserted in their mangled form, a reference will be added to the demangled overload set

	struct VarData {
		int off;
		std::optional<Location> src;
		bool is_mut = false;
	};

	// TODO: Consider subtyping from `VarData` (if allowed in variants)
	// This should allow code which treats functions as variables to do so without modification
	// Plus many of the stuff that functions and variables currently consider are the same
	// NOTE: Individual functions will be inserted into the symbol table under their mangled name
	// The overload set partially serves to act as an index point for the demangled functions
	// TODO: Where should the resolution interfaces belong, in the vardata or in the overloadset ?
	struct OverloadSet {
		//std::vector<std::string> mangled_names;
		std::vector<ref_t<DataType>> mangled_fns;
	};

	//class SymTable : public std::unordered_map<std::string, DataSet> {
	class SymTable {
		SymTable* parent = nullptr;
		std::unordered_map<std::string, DataType> vars;
		//std::vector<SymTable*> imported = { this };

		public:
			int ebp_offset = 0;

			// Search for a value at the current scoping level
			// NOTE: Accessing through '[]' will not search in parent scopes
			ref_t<DataType> operator[](std::string key);
			bool exists(std::string key);

			ref_t<DataType> insert(std::string key, DataType value);
			std::optional<ref_t<DataType>> insertIfNew(std::string key, DataType value);

			// TODO: Work on interface
			// TODO: Implement
			//void importScope(SymTable* scope);

			// TODO: Might need a way to differentiate between queries which should index parents and queries which shouldn't
			std::optional<ref_t<DataType>> get(std::string key);
			std::optional<ref_t<VarData>> getVar(std::string key);
			std::optional<ref_t<SymTable>> getScope(std::string key);
			std::optional<ref_t<OverloadSet>> getFunc(std::string key);

			// Information querying
			size_t size();
			size_t numVariables();

			// Hierarchy setup
			void setParent(SymTable* p, bool offset_ebp = false);
			SymTable* getParent();
	};

}
