#pragma once

#include <unordered_map>
#include <optional>
#include <variant>

#include "parser/location.h"
#include "util/analysis.h"

template<class T>
using opt_t = std::optional<T>;
template<class T>
using ref_t = std::reference_wrapper<T>;

namespace spero::compiler::analysis {
	/*
     * Specifies the data relevant for analysis of Spero variables
	 */
	struct VarData {
		Location src;
		bool is_mut;

		analysis::memory::Locations storage;

		// TODO: Not sure about these (using too many pointers)
		//ast::Ast* definition = nullptr;
		//std::vector<ast::Ast*> usages;

		VarData(Location s, bool mut) : VarData{ s, mut, analysis::memory::Stack{} } {}
		VarData(Location s, bool mut, analysis::memory::Locations loc) : src{ s }, is_mut{ mut }, storage{ loc } {}
	};

	/*
	 * Collect all definitions for a single symbol name under a unified group for some analysis steps
	 *   This structure performs the dual roles of ssa name resolution function overloading
	 */
	struct SsaVector : std::vector<VarData> {
		bool is_overload_set;
	};

	/*
	 * Stores the mapping between symbol names and the needed information for analysis resolution
	 *   Aside from `mostRecentDef`, all interfaces only interact with the internal "level" of resolution
	 *   The interfaces do not try to search for a definition within a parent SymTable
	 *
	 * TODO: See how well this works with type specialization and function overloading
	 */
	class SymTable {
		public:
			using StorageType = std::variant<SsaVector, ref_t<SymTable>>;
			using DataType = std::variant<ref_t<VarData>, ref_t<SymTable>>;

		private:
			SymTable * parent = nullptr;

			std::unordered_map<std::string, StorageType> vars;
			ScopingContext rules = ScopingContext::SCOPE;

		public:
			int curr_ebp_offset = 0;
			SymTable();
			
			// Basic accessors and queries
			ref_t<StorageType> operator[](std::string key);
			bool exists(std::string key) const;

			// Accessor interfaces
			opt_t<ref_t<StorageType>> get(std::string key);
			opt_t<ref_t<SymTable>> getScope(std::string key);
			opt_t<ref_t<SsaVector>> getOverloadSet(std::string key);
			opt_t<ref_t<VarData>> getVariable(std::string key, size_t ssa_id);
			opt_t<DataType> ssaIndex(std::string key, opt_t<size_t>& index, const Location& loc);

			// Mutation interfaces
			bool insert(std::string key, VarData value);
			// TODO: There's an extra failure case of a symtable already existing
			bool insert(std::string key, ref_t<SymTable> value);

			// Analysis interfaces
			void setParent(SymTable* p, bool offset_ebp = false);
			SymTable* mostRecentDef(std::string key);
			void setContextRules(ScopingContext rule);
			ScopingContext getContextRules() const;

			// Counting interfaces
			size_t size() const;
			// TODO: Need to rewrite to account for ssa (and drop scopes)
			size_t numVariables();
			size_t count(std::string key) const;
	};

}
