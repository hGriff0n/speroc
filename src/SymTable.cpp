#include "analysis/SymTable.h"

#include <numeric>

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

	std::optional<ref_t<DataType>> SymTable::get(std::string key) {
		if (exists(key)) {
			return operator[](key);
		}

		return {};
	}

	std::optional<ref_t<VarData>> SymTable::getVar(std::string key) {
		if (exists(key)) {
			auto& var = operator[](key);

			if (std::holds_alternative<VarData>(var.get())) {
				return std::get<VarData>(var.get());
			}
		}

		return {};
	}

	std::optional<ref_t<SymTable>> SymTable::getScope(std::string key) {
		if (exists(key)) {
			auto& var = operator[](key);

			if (std::holds_alternative<ref_t<SymTable>>(var.get())) {
				return std::get<ref_t<SymTable>>(var.get());
			}
		}

		return {};
	}

	std::optional<ref_t<OverloadSet>> SymTable::getFunc(std::string key) {
		if (exists(key)) {
			auto& var = operator[](key);

			if (std::holds_alternative<OverloadSet>(var.get())) {
				return std::get<OverloadSet>(var.get());
			}
		}

		return {};
	}

	SymTable* SymTable::mostRecentDef(std::string key) {
		auto* scope = this;

		while (scope && !scope->exists(key)) {
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

	size_t SymTable::size() {
		return vars.size();
	}
	size_t SymTable::numVariables() {
		return std::accumulate(vars.begin(), vars.end(), 0,
			[](int acc, auto& var) { return acc + std::holds_alternative<VarData>(var.second); });
	}

	void SymTable::setParent(SymTable* p, bool offset_ebp) {
		parent = p;
		insert("super", *p);
		ebp_offset = offset_ebp * (p->ebp_offset + (p->numVariables() * 4));
	}

}
