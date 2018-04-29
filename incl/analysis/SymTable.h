#pragma once

#include <unordered_map>
#include <optional>
#include <variant>

#include "parser/location.h"
#include "util/analysis.h"

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
		std::optional<Location> src;
		// std::variant<Stack, Heap, Offset, Global, Static> storage;

		int off;
		bool is_global = false;

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

	/*
     * Specifies the data relevant for analysis of Spero variables
	 */
	struct _VarData {
		Location src;
		bool is_mut;

		analysis::memory::Locations storage;

		// TODO: Not sure about these (using too many pointers)
		//ast::Ast* definition = nullptr;
		//std::vector<ast::Ast*> usages;

		_VarData(Location s, bool mut) : _VarData{ s, mut, analysis::memory::Stack{} } {}
		_VarData(Location s, bool mut, analysis::memory::Locations loc) : src{ s }, is_mut{ mut }, storage{ loc } {}
	};

	/*
	 * Collect all definitions for a single symbol name under a unified group for some analysis steps
	 *   This structure performs the dual roles of ssa name resolution function overloading
	 */
	struct SsaVector : std::vector<_VarData> {
		bool is_overload_set;
	};

	template<class T>
	using opt = std::optional<T>;
	template<class T>
	using ref = std::reference_wrapper<T>;

	/*
	 * Stores the mapping between symbol names and the needed information for analysis resolution
	 *   Aside from `mostRecentDef`, all interfaces only interact with the internal "level" of resolution
	 *   The interfaces do not try to search for a definition within a parent SymTable
	 *
	 * TODO: See how well this works with type specialization and function overloading
	 */
	class _SymTable {
		public:
			using StorageType = std::variant<SsaVector, ref<_SymTable>>;
			using DataType = std::variant<ref<_VarData>, ref<_SymTable>>;

		private:
			_SymTable* parent = nullptr;

			std::unordered_map<std::string, StorageType> vars;
			ScopingContext rules = ScopingContext::SCOPE;

		public:
			int curr_ebp_offset = 0;
			_SymTable();
			
			// Basic accessors and queries
			ref<StorageType> operator[](std::string key);
			bool exists(std::string key) const;

			// Accessor interfaces
			opt<ref<StorageType>> get(std::string key);
			opt<ref<_SymTable>> getScope(std::string key);
			opt<ref<SsaVector>> getOverloadSet(std::string key);
			opt<ref<_VarData>> getVariable(std::string key, size_t ssa_id);
			opt<DataType> ssaIndex(std::string key, opt<size_t>& index, const Location& loc);

			// Mutation interfaces
			bool insert(std::string key, _VarData value);
			// TODO: There's an extra failure case of a symtable already existing
			bool insert(std::string key, ref<_SymTable> value);

			// Analysis interfaces
			void setParent(_SymTable* p, bool offset_ebp = false);
			_SymTable* mostRecentDef(std::string key);
			void setContextRules(ScopingContext rule);
			ScopingContext getContextRules() const;

			// Counting interfaces
			size_t size() const;
			// TODO: Need to rewrite to account for ssa (and drop scopes)
			size_t numVariables();
			size_t count(std::string key) const;
	};

	//class SymTable : public std::unordered_map<std::string, DataType> {
	class SymTable {
		SymTable* parent = nullptr;
		std::unordered_map<std::string, DataType> vars;
		std::unordered_map<std::string, std::pair<std::string, size_t>> ssa_map;
		//std::vector<SymTable*> imported = { this };

		public:
			int ebp_offset = 0;

			SymTable();

			// Search for a value at the current scoping level
			ref_t<DataType> operator[](std::string key);
			bool exists(std::string key);
			bool hasSSA(std::string key);

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

			// SSA interface
			std::string allocateName(std::string key);
			std::optional<std::string> getSSA(std::string key);

			// Information querying
			size_t size();
			size_t numVariables();
			size_t count(std::string key);
	};

}
