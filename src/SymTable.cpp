#include "analysis/SymTable.h"

#include <algorithm>
#include <numeric>
#include <string>

namespace spero::compiler::analysis {

	_SymTable::_SymTable() {
		insert("self", *this);
	}

	ref_t<DataType> _SymTable::operator[](std::string key) {
		// Option 3: Scan through imported tables for definition
			// Create in calling symtable if not found
		/*for (auto* table : imported) {
			if (table->exists(key)) {
				return table->vars[key];
			}
		}*/

		return vars[key];
	}

	bool _SymTable::exists(std::string key) {
		return vars.count(key) != 0;
	}

	bool _SymTable::hasSSA(std::string key) {
		return ssa_map.count(key) != 0;
	}

	std::optional<ref_t<DataType>> _SymTable::get(std::string key) {
		if (exists(key)) {
			return operator[](key);
		}

		return {};
	}

	std::optional<ref_t<VarData>> _SymTable::getVar(std::string key) {
		if (exists(key)) {
			auto& var = operator[](key);

			if (auto* data = std::get_if<VarData>(&var.get())) {
				return *data;
			}
		}

		return {};
	}

	std::optional<ref_t<_SymTable>> _SymTable::getScope(std::string key) {
		if (exists(key)) {
			auto& var = operator[](key);

			if (auto* data = std::get_if<ref_t<_SymTable>>(&var.get())) {
				return *data;
			}
		}

		return {};
	}

	std::optional<ref_t<OverloadSet>> _SymTable::getFunc(std::string key) {
		if (exists(key)) {
			auto& var = operator[](key);

			if (auto* data = std::get_if<OverloadSet>(&var.get())) {
				return *data;
			}
		}

		return {};
	}

	_SymTable* _SymTable::mostRecentDef(std::string key) {
		auto* scope = this;

		while (scope && !scope->exists(key) && !scope->hasSSA(key)) {
			scope = scope->parent;
		}

		return scope;
	}

	ref_t<DataType> _SymTable::insert(std::string key, DataType value) {
		return operator[](key).get() = value;
	}

	std::optional<ref_t<DataType>> _SymTable::insertIfNew(std::string key, DataType value) {
		if (exists(key)) {
			return insert(key, value);
		}

		return {};
	}

	std::optional<DataType> _SymTable::remove(std::string key) {
		auto elem = get(key);
		vars.erase(key);

		if (elem) {
			return elem->get();
		}

		return std::nullopt;
	}

	std::string _SymTable::allocateName(std::string key) {
		auto& ssa = ssa_map[key];
		return (ssa.first = key + '$' + std::to_string(ssa.second++));
	}

	std::optional<std::string> _SymTable::getSSA(std::string key) {
		if (ssa_map.count(key) != 0) {
			return ssa_map[key].first;
		}

		return {};
	}

	size_t _SymTable::size() {
		return vars.size();
	}
	size_t _SymTable::numVariables() {
		return std::accumulate(vars.begin(), vars.end(), 0,
			[](int acc, auto& var) { return acc + std::holds_alternative<VarData>(var.second); });
	}

	size_t _SymTable::count(std::string key) {
		return vars.count(key);
	}

	void _SymTable::setParent(_SymTable* p, bool offset_ebp) {
		insert("super", *(parent = p));
		ebp_offset = offset_ebp * (p->ebp_offset + (p->numVariables() * 4));
	}



	SymTable::SymTable() {
		insert("self", *this);
	}

	// Basic accessors and queries
	ref<SymTable::StorageType> SymTable::operator[](std::string key) {
		return vars[key];
	}
	bool SymTable::exists(std::string key) const {
		return vars.count(key) != 0;
	}

	// Accessor interfaces
	opt<ref<SymTable::StorageType>> SymTable::get(std::string key) {
		if (exists(key)) {
			return operator[](key);
		}

		return std::nullopt;
	}
	opt<ref<SymTable>> SymTable::getScope(std::string key) {
		if (auto data = get(key)) {
			if (auto* table = std::get_if<ref<SymTable>>(&data->get())) {
				return table->get();
			}
		}

		return std::nullopt;
	}
	opt<ref<SsaVector>> SymTable::getOverloadSet(std::string key) {
		if (auto data = get(key)) {
			if (auto* vec = std::get_if<SsaVector>(&data->get())) {
				return *vec;
			}
		}

		return std::nullopt;
	}
	opt<ref<_VarData>> SymTable::getVariable(std::string key, size_t ssa_id) {
		if (auto ssa = getOverloadSet(key); ssa->get().size() > ssa_id) {
			return ssa->get()[ssa_id];
		}

		return std::nullopt;
	}
	opt<SymTable::DataType> SymTable::ssaIndex(std::string key, opt<size_t>& index, const Location& loc) {
		if (auto data = get(key)) {
			if (auto* vec = std::get_if<SsaVector>(&data->get())) {
				if (!index) {
					auto ssa_index = std::distance(vec->begin(), 
						std::upper_bound(vec->begin(), vec->end(), loc,
							[&](auto&& use_loc, auto&& dec_loc) {
								return dec_loc.src.line_num < use_loc.line_num
									&& dec_loc.src.byte < use_loc.byte;
							}));

					if (ssa_index != 0) {
						return (*vec)[*(index = ssa_index - 1)];

					} else if (getContextRules() != +ScopingContext::SCOPE) {
						return (*vec)[*(index = ssa_index)];
					}
				} else {
					return (*vec)[*index];
				}

			} else {
				return std::get<ref<SymTable>>(data->get());
			}
		}

		return std::nullopt;
	}

	// Mutation interfaces
	bool SymTable::insert(std::string key, _VarData value) {
		if (!exists(key)) {
			vars[key] = SsaVector{};
		}

		if (auto var = getOverloadSet(key)) {
			var->get().emplace_back(std::move(value));
			return true;
		}

		return false;
	}
	bool SymTable::insert(std::string key, ref<SymTable> value) {
		if (!exists(key)) {
			vars[key] = value.get();
			return true;
		}

		return false;
	}

	// Analysis interfaces
	void SymTable::setParent(SymTable* p, bool offset_ebp) {
		insert("super", *(parent = p));
		curr_ebp_offset = offset_ebp * (p->curr_ebp_offset + (p->numVariables() * 4));
	}
	SymTable* SymTable::mostRecentDef(std::string key) {
		auto* scope = this;

		while (scope && !scope->exists(key)) {
			scope = scope->parent;
		}

		return scope;
	}
	void SymTable::setContextRules(ScopingContext rule) {
		rules = rule;
	}
	ScopingContext SymTable::getContextRules() const {
		return rules;
	}

	// Counting interfaces
	size_t SymTable::size() const {
		return vars.size();
	}
	size_t SymTable::numVariables() {
		return std::accumulate(vars.begin(), vars.end(), 0, [](size_t acc, auto& var) {
			if (auto* vec = std::get_if<SsaVector>(&var.second)) {
				acc += vec->size();
			}
			return acc;
		});
	}
	size_t SymTable::count(std::string key) const {
		return vars.count(key);
	}

}
