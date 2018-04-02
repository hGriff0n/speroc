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

	// NOTE: All symbol tables are referenced from an external source (particularly necessary for 'self')
	using DataType = std::variant<VarData, OverloadSet, ref_t<SymTable>>;


	// TODO: How should importing a module be handled in regards to all of its information (leave until I implement modules)
		// IDEA: Imports are going to take the form of explicitly assigning a reference/pointer of the imported object to the imported name
			// "use std:io" will create an entry directing towards "std:io" under the name "io" in the current scope
		// TODO: How to fucking type and interface that ??
			// You also have to make using an imported variable indistinct from using a declared variable

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
		// TODO: What should the data in the mangled functions be ???
	// TODO: Where should the resolution interfaces belong, in the vardata or in the overloadset ?
	struct OverloadSet {
		//std::vector<std::string> mangled_names;
		std::vector<ref_t<DataType>> mangled_fns;
	};

	//class SymTable : public std::unordered_map<std::string, DataType> {
	class SymTable {
		SymTable* parent = nullptr;
		std::unordered_map<std::string, DataType> vars;
		//std::vector<SymTable*> imported = { this };

		public:
			int ebp_offset = 0;

			SymTable();

			// Search for a value at the current scoping level
			ref_t<DataType> operator[](std::string key);
			bool exists(std::string key);

			// Accessor wrappers for accessing at the current scoping level
			std::optional<ref_t<DataType>> get(std::string key);
			std::optional<ref_t<VarData>> getVar(std::string key);
			std::optional<ref_t<SymTable>> getScope(std::string key);
			std::optional<ref_t<OverloadSet>> getFunc(std::string key);

			// Slightly simpler insertion interface (don't need to dereference `ref_t`)
			ref_t<DataType> insert(std::string key, DataType value);
			std::optional<ref_t<DataType>> insertIfNew(std::string key, DataType value);

			// Returns the most recent `SymTable` which has a definition for the passed key
			SymTable* mostRecentDef(std::string key);

			// TODO: Import interface
			//void importScope(SymTable* scope);

			// Hierarchy setup
			void setParent(SymTable* p, bool offset_ebp = false);

			// Information querying
			size_t size();
			size_t numVariables();
			size_t count(std::string key);
	};

}
