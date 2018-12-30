#pragma once

#include <deque>
#include <optional>
#include <set>
#include <unordered_map>
#include <variant>

#include "parser/base.h"
#include "util/analysis.h"

template<class T>
using opt_t = std::optional<T>;
template<class T>
using ref_t = std::reference_wrapper<T>;
//TODO: Create `opt_ref` struct to wrap 'ref_t' in 'opt_t' ???

namespace llvm {
	class Value;
}

namespace spero::analysis {
	class Type;
	using SymIndex = size_t;

	/*
     * Specifies the data relevant for analysis of Spero variables
	 */
	struct SymbolInfo {
		// Location of the variable's definition in the source code
		compiler::Location src;

		// Whether the variable was declared mutable or not
		// NOTE: This is a constant, but that deletes the `operator=` which is needed by `std::deque`
		bool is_mut;

		// Index into the SymTable arena that stores the type's definition
		// NOTE: This is only not `std::nullopt` when the symbol defines a spero type
		opt_t<SymIndex> type_def_index = std::nullopt;

		// The symbol's inferred type scheme
		std::shared_ptr<Type> type = nullptr;

		// TODO: Not sure if we should have this
		compiler::ast::Ast* definition = nullptr;

		// Llvm allocated storage location
		llvm::Value* storage = nullptr;
		// TODO: How would types be handled? 
	};

	/*
	 * Collect all definitions for a single symbol name under a unified group for some analysis steps
	 *   This structure performs the dual roles of ssa name resolution function overloading
	 */
	struct SsaVector : std::deque<SymbolInfo> {
		bool is_overload_set;
	};

	/*
	 * Struct for specifying an "imported" table that contains the actual symbol definition
	 *   In practice, this means re-starting the lookup process in the linked table at the current name
	 *
	 * "use std:io" -> creates a Redirect entry under "io" to point to the 'std' SymTable
	 * "use std:io:_" -> creates Redirect entries for every definition exported by 'std:io' to point to the 'std:io' SymTable
	 */
	struct Redirect {
		SymIndex lookup;
		//bool glob = false;
	};

	/*
	 * Struct for resolving generic definitions before monomorphization
	 *
	 * TODO: This may not be needed depending on how generics will be handled
	 */
	using GenericInstanceIndexType = void*;
	class GenericResolver {
		public:
			using SymbolTypes = std::variant<Redirect, SymIndex, ref_t<SsaVector>>;
			using SymbolInputTypes = std::variant<Redirect, SymIndex, SsaVector>;

		private:
			std::unordered_map<GenericInstanceIndexType, SymbolInputTypes> instances;

			// This member is solely used in the case of importing, particularly "import all"
			// At that stage, since we are using the 'Redirect' struct mechanism to implement
			// The rebinding, our only care is whether the symbol under consideration requires
			// The redirection (ie. whether it is exported). Since this type is responsible for
			// Handling all definitions under a single symbol, we want to maintain that information
			// Here, so that we don't have to go looking for it throughout the map. If we are
			// Instead considering whether a specific *definition* is exported or not, then we
			// Can find that information within the definition's SymbolInfo/etc. as we should
			// Already be accessing that data by the time that we come to check on it
			bool has_at_least_one_exported_definition = false;

		public:
			opt_t<SymbolTypes> resolve(GenericInstanceIndexType index);

			// These only fail if a binding under the key already exists
			// And the existing symbol has a different "character" than the new symbol
			// ie. you can't have a 'Redirect' and 'SymbolInfo' under the same name
			bool set(GenericInstanceIndexType index, SymbolInputTypes data);
			bool set(GenericInstanceIndexType index, SymbolInfo data);

			// Visibility interfaces
			bool exported();
			void markExported();
	};
	

	/*
	 * Stores the mapping between symbol names and the needed information for analysis resolution
	 *   Aside from `mostRecentDef`, all interfaces only interact with the internal "level" of resolution
	 *   The interfaces do not try to search for a definition within a parent SymTable
	 *
	 * NOTE: This may not be the way we want to go moving forward
	 *   This implementation handles the ssa lookup after the generics have been "monomorphized"
	 *   However, in the context of type inference, this "monomorphization", may depend on the ssa lookup
	 *   This may depend on how we handle the monomorphization process, we can turn `GenericResolver` away from the map for one
	 */
	class SymTable {
		public:
			using SymbolTypes = std::variant<Redirect, SymIndex, ref_t<SymbolInfo>>;

		private:
			SymIndex self_index;
			opt_t<SymIndex> parent;
			std::unordered_map<String, GenericResolver> symbols;
			std::set<String> argument_set;

			analysis::ScopingContext scope_context;

		public:
			SymTable(SymIndex self, analysis::ScopingContext context=analysis::ScopingContext::SCOPE);
			
			// Basic accessors and queries
			ref_t<GenericResolver> operator[](const String& key);
			bool exists(const String& key) const;

			// Accessor interfaces
			opt_t<ref_t<GenericResolver>> get(const String& key);
			opt_t<SymbolTypes> get(const String& key, GenericInstanceIndexType index, compiler::Location loc, opt_t<size_t>& ssa_index);

 			// Mutation interfaces
			bool insert(const String& key, SymbolInfo value, bool exported = false);
			bool insert(const String& key, GenericResolver::SymbolInputTypes value, bool exported = false);
			bool insertArg(const String& key, SymbolInfo value);
			bool insertArg(const String& key, GenericResolver::SymbolInputTypes value);

			// Analysis interfaces
			SymIndex self() const;
			void setParent(SymIndex p);
			void setContext(ScopingContext rule);
			ScopingContext context() const;
			SymTable* mostRecentDef(const String& key, std::deque<SymTable>& arena);

			// Counting interfaces
			inline const auto begin() const {
				return symbols.cbegin();
			}
			inline const auto end() const {
				return symbols.cend();
			}
			inline const auto arguments() const {
				return std::make_pair(argument_set.cbegin(), argument_set.cend());
			}
	};


	// Helper functions
	using SymArena = std::deque<SymTable>;
	constexpr SymIndex GLOBAL_SYM_INDEX = 0;

	inline size_t numArgs(const SymArena& arena, SymIndex table) {
		auto[front, end] = arena[table].arguments();
		return std::distance(front, end);
	}

}
