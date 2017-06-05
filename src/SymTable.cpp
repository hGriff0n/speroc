#include "SymTable.h"

namespace spero::compiler::analysis {

	SymTable::SymTable() {}

	SymTable::SymTable(SymTable* parent) {}

	SymTable::~SymTable() {}

	void SymTable::insert(std::string name, int off) {
		// TODO: Add in check against "shadowing" ???
		var_data[name].loc = off;
	}

	std::optional<int> SymTable::getVar(std::string name) {
		if (var_data.count(name)) {
			return var_data[name].loc;
		}

		return {};
	}

	size_t SymTable::getCount() {
		return var_data.size();
	}

}
