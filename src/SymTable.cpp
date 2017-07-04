#include "SymTable.h"

namespace spero::compiler::analysis {

	SymTable::SymTable() {}

	SymTable::~SymTable() {}

	void SymTable::insert(std::string name, int off) {
		// TODO: Add in check against "shadowing" ???
		var_data[name].loc = off - ebp_offset;
	}

	std::optional<int> SymTable::getVar(std::string name) {
		if (var_data.count(name)) {
			return var_data[name].loc;
		}

		if (parent) {
			return parent->getVar(name);
		}

		return {};
	}

	size_t SymTable::getCount() {
		return var_data.size();
	}

	void SymTable::setParent(SymTable* p, bool offset_ebp) {
		parent = p;
		ebp_offset = offset_ebp * p->getCount() * 4;
	}

	SymTable* SymTable::getParent() {
		return parent;
	}
}
