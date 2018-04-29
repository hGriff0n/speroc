#include "analysis/SymTable.h"

#include <algorithm>
#include <numeric>
#include <string>

namespace spero::compiler::analysis {

	SymTable::SymTable() {
		insert("self", *this);
	}

	// Basic accessors and queries
	ref_t<SymTable::StorageType> SymTable::operator[](std::string key) {
		return vars[key];
	}
	bool SymTable::exists(std::string key) const {
		return vars.count(key) != 0;
	}

	// Accessor interfaces
	opt_t<ref_t<SymTable::StorageType>> SymTable::get(std::string key) {
		if (exists(key)) {
			return operator[](key);
		}

		return std::nullopt;
	}
	opt_t<ref_t<SymTable>> SymTable::getScope(std::string key) {
		if (auto data = get(key)) {
			if (auto* table = std::get_if<ref_t<SymTable>>(&data->get())) {
				return table->get();
			}
		}

		return std::nullopt;
	}
	opt_t<ref_t<SsaVector>> SymTable::getOverloadSet(std::string key) {
		if (auto data = get(key)) {
			if (auto* vec = std::get_if<SsaVector>(&data->get())) {
				return *vec;
			}
		}

		return std::nullopt;
	}
	opt_t<ref_t<VarData>> SymTable::getVariable(std::string key, size_t ssa_id) {
		if (auto ssa = getOverloadSet(key); ssa->get().size() > ssa_id) {
			return ssa->get()[ssa_id];
		}

		return std::nullopt;
	}
	opt_t<SymTable::DataType> SymTable::ssaIndex(std::string key, opt_t<size_t>& index, const Location& loc) {
		if (auto data = get(key)) {
			if (auto* vec = std::get_if<SsaVector>(&data->get())) {
				if (!index) {
					auto ssa_index = std::distance(vec->begin(), 
						std::upper_bound(vec->begin(), vec->end(), loc,
							[&](auto&& use_loc, auto&& dec_loc) -> bool {
								if (use_loc.line_num == dec_loc.src.line_num) {
									return use_loc.byte < dec_loc.src.byte;
								}

								return use_loc.line_num < dec_loc.src.line_num;
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
				return std::get<ref_t<SymTable>>(data->get());
			}
		}

		return std::nullopt;
	}

	// Mutation interfaces
	bool SymTable::insert(std::string key, VarData value) {
		if (!exists(key)) {
			vars[key] = SsaVector{};
		}

		if (auto var = getOverloadSet(key)) {
			var->get().emplace_back(std::move(value));
			return true;
		}

		return false;
	}
	bool SymTable::insert(std::string key, ref_t<SymTable> value) {
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
