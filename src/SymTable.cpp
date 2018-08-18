#include "analysis/SymTable.h"

#include <algorithm>
#include <numeric>
#include <string>

namespace spero::analysis {

	opt_t<GenericResolver::SymbolTypes> GenericResolver::resolve(GenericInstanceIndexType index) {
		if (instances.count(index)) {
			return std::visit([](auto&& var) -> opt_t<SymbolTypes> {
				using VarType = std::decay_t<decltype(var)>;
				if constexpr (std::is_same_v<VarType, SsaVector>)  {
					return var;
				}

				if constexpr (std::is_same_v<VarType, Redirect> || std::is_same_v<VarType, SymIndex>) {
					return var;
				}

				return std::nullopt;
			}, instances.at(index));
		}

		return std::nullopt;
	}

	bool GenericResolver::set(GenericInstanceIndexType index, SymbolInputTypes data) {
		if (instances.count(index)) {
			if (instances[index].index() != data.index()) {
				return false;
			}
		}

		instances[index] = data;
		return true;
	}

	bool GenericResolver::set(GenericInstanceIndexType index, SymbolInfo data) {
		// Handle in-scope shadowing definitions
		if (instances.count(index)) {
			if (auto vec = std::get_if<SsaVector>(&instances[index])) {
				vec->push_back(data);
				return true;
			}

			// Existing definition of wrong type
			return false;
		}

		// Otherwise make a new definition
		SsaVector decl;
		decl.push_back(data);
		instances[index] = decl;
		return true;
	}

	bool GenericResolver::exported() {
		return has_at_least_one_exported_definition;
	}
	void GenericResolver::markExported() {
		has_at_least_one_exported_definition = true;
	}

	SymTable::SymTable(SymIndex self) : self_index{ self } {
		insert("self", self);
	}

	// Basic accessors and queries
	ref_t<GenericResolver> SymTable::operator[](const String& key) {
		return symbols[key];
	}
	bool SymTable::exists(const String& key) const {
		return symbols.count(key) != 0;
	}

	// Accessor interfaces
	opt_t<ref_t<GenericResolver>> SymTable::get(const String& key) {
		if (exists(key)) {
			return symbols.at(key);
		}

		return std::nullopt;
	}

	opt_t<SymTable::SymbolTypes> SymTable::get(const String& key, GenericInstanceIndexType index, compiler::Location loc) {
		// Drill down to the resolved instance
		if (auto gen = get(key)) {
			if (auto res = gen->get().resolve(index)) {
			// Extract out the specific `SymbolInfo` struct if it exists
				if (auto ssa_ref = std::get_if<ref_t<SsaVector>>(&*res)) {
					auto ssa = &ssa_ref->get();
					auto next_dec = std::upper_bound(ssa->begin(), ssa->end(), loc,
						[&](auto&& use_loc, auto&& dec_loc) -> bool {
							if (use_loc.line_num == dec_loc.src.line_num) {
								return use_loc.byte < dec_loc.src.byte;
							}

							return use_loc.line_num < dec_loc.src.line_num;
						});

					if (next_dec != ssa->begin()) {
						return *(next_dec - 1);

					} else if (scope_context != +ScopingContext::SCOPE) {
						return *next_dec;
					}

				// Otherwise, convert the expected types
				} else if (auto redirect = std::get_if<Redirect>(&*res)) {
					return *redirect;
				} else if (auto index = std::get_if<SymIndex>(&*res)) {
					return *index;
				}
			}
		}

		return std::nullopt;
	}

	// Mutation interfaces
	bool SymTable::insert(const String& key, SymbolInfo value, bool exported) {
		if (exported) {
			symbols[key].markExported();
		}

		return symbols[key].set(nullptr, value);
	}
	bool SymTable::insert(const String& key, GenericResolver::SymbolInputTypes value, bool exported) {
		if (exported) {
			symbols[key].markExported();
		}

		return symbols[key].set(nullptr, value);
	}
	bool SymTable::insertArg(const String& key, SymbolInfo value) {
		if (!exists(key)) {
			argument_set.insert(key);

			return insert(key, value);
		}

		return false;
	}
	bool SymTable::insertArg(const String& key, GenericResolver::SymbolInputTypes value) {
		if (!exists(key)) {
			argument_set.insert(key);

			return insert(key, value);
		}

		return false;
	}

	// Analysis interfaces
	SymIndex SymTable::self() const {
		return self_index;
	}
	SymTable* SymTable::mostRecentDef(const String& key, SymArena& arena) {
		SymTable* scope = this;

		while (scope && !scope->exists(key)) {
			if (!scope->parent.has_value()) {
				scope = nullptr;
			} else {
				scope = &arena.at(*scope->parent);
			}
		}

		return scope;
	}
	void SymTable::setParent(SymIndex p) {
		insert("super", p);
		parent = p;
	}
	void SymTable::setContext(ScopingContext rule) {
		scope_context = rule;
	}
	ScopingContext SymTable::context() const {
		return scope_context;
	}

}
