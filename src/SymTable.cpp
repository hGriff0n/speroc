#include "analysis/SymTable.h"

#include <numeric>

namespace spero::compiler::analysis {

	SymTable::DataType& SymTable::operator[](std::string key) {
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

	// TODO: Make sure modifications of ret type work
	std::optional<SymTable::DataType> SymTable::get(std::string key) {
		if (exists(key)) {
			return operator[](key);
		}

		if (parent) {
			return parent->get(key);
		}

		return {};
	}

	void SymTable::insert(std::string key, DataType value) {
		operator[](key) = value;
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
		ebp_offset = offset_ebp * (p->ebp_offset + (p->numVariables() * 4));
	}

	SymTable* SymTable::getParent() {
		return parent;
	}

}
