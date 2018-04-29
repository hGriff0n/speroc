#include "analysis/SymTable.h"

#include <numeric>
#include <string>

namespace spero::compiler::analysis {

	SymTable::SymTable() {
		insert("self", *this);
	}

	ref_t<DataType> SymTable::operator[](std::string key) {
		// Option 3: Scan through imported tables for definition
			// Create in calling symtable if not found
		/*for (auto* table : imported) {
			if (table->exists(key)) {
				return table->vars[key];
			}
		}*/

		return vars[key];
	}

	bool SymTable::exists(std::string key) {
		return vars.count(key) != 0;
	}

	bool SymTable::hasSSA(std::string key) {
		return ssa_map.count(key) != 0;
	}

	std::optional<ref_t<DataType>> SymTable::get(std::string key) {
		if (exists(key)) {
			return operator[](key);
		}

		return {};
	}

	std::optional<ref_t<VarData>> SymTable::getVar(std::string key) {
		if (exists(key)) {
			auto& var = operator[](key);

			if (auto* data = std::get_if<VarData>(&var.get())) {
				return *data;
			}
		}

		return {};
	}

	std::optional<ref_t<SymTable>> SymTable::getScope(std::string key) {
		if (exists(key)) {
			auto& var = operator[](key);

			if (auto* data = std::get_if<ref_t<SymTable>>(&var.get())) {
				return *data;
			}
		}

		return {};
	}

	std::optional<ref_t<OverloadSet>> SymTable::getFunc(std::string key) {
		if (exists(key)) {
			auto& var = operator[](key);

			if (auto* data = std::get_if<OverloadSet>(&var.get())) {
				return *data;
			}
		}

		return {};
	}

	SymTable* SymTable::mostRecentDef(std::string key) {
		auto* scope = this;

		while (scope && !scope->exists(key) && !scope->hasSSA(key)) {
			scope = scope->parent;
		}

		return scope;
	}

	ref_t<DataType> SymTable::insert(std::string key, DataType value) {
		return operator[](key).get() = value;
	}

	std::optional<ref_t<DataType>> SymTable::insertIfNew(std::string key, DataType value) {
		if (exists(key)) {
			return insert(key, value);
		}

		return {};
	}

	std::string SymTable::allocateName(std::string key) {
		auto& ssa = ssa_map[key];
		return (ssa.first = key + '$' + std::to_string(ssa.second++));
	}

	std::optional<std::string> SymTable::getSSA(std::string key) {
		if (ssa_map.count(key) != 0) {
			return ssa_map[key].first;
		}

		return {};
	}

	size_t SymTable::size() {
		return vars.size();
	}
	size_t SymTable::numVariables() {
		return std::accumulate(vars.begin(), vars.end(), 0,
			[](int acc, auto& var) { return acc + std::holds_alternative<VarData>(var.second); });
	}

	size_t SymTable::count(std::string key) {
		return vars.count(key);
	}

	void SymTable::setParent(SymTable* p, bool offset_ebp) {
		insert("super", *(parent = p));
		ebp_offset = offset_ebp * (p->ebp_offset + (p->numVariables() * 4));
	}

}
