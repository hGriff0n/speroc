#pragma once

#include <unordered_map>
#include <optional>
#include <variant>

#include "parser/location.h"

namespace spero::compiler::analysis {
	// There are always going to be 2 symbol tables: global and current scopes
		// This allows us to go both ways for finding a symbol name
	// Tables will implicitly support single step indexing to get a symbol table or variable data structure
		// ie. operator[](string) -> variant<SymTable, VarData, FunctionSet?>
		// TODO: How do we handle functions, particularly function overloading
			// VarData at the moment is implicitly one-to-one, we'll probably have to create a different class
			// Could use a std::multimap, but I don't like how that'll work for normal variables
			// Other option is to have a function set type, force function handling code to handle the overloads
		// TODO: Come up with a quick way to insert/get a qualified variable
			// get should always fail if any step on the way doesn't exist
			// insertion should probably fail as well
				// the only time where this behavior wouldn't make sense is in nested modules
				// however, i can special case that or handle it manually
			// However this isn't the domain of the SymTable, I just need to handle one step here
	// I still need the parent interfaces in order to handle reverse indexing
		// This probably shouldn't be handled through the 'operator[]' interface
	// TODO: How should importing a module be handled in regards to all of its information (leave until I implement modules)
		// Option 1: Copy everything over
		// Option 2: Have a set of SymTable*
		// Option 3: Extend the SymTable* set idea, but have myself be a part of that interface
			// This should simplify the implementation code a bit, but is conceptually harder to explain
			// Might also enable easier detection of name clashes due to importing (TODO: Decide how that must be handled)
				// Though I think having the current name be the default is an alright semantics (must access imported through qualified name)

	struct VarData {
		int off;
		std::optional<Location> src;
		bool is_mut;
	};

	struct OverloadSet {};

	//class SymTable : public std::unordered_map<std::string, std::variant<VarData, _SymTable, OverloadSet>> {
	class SymTable {
		using DataType = std::variant<VarData, SymTable, OverloadSet>;			// I'll finally have a use for that one util file !!!

		SymTable* parent = nullptr;
		std::unordered_map<std::string, DataType> vars;
		//std::vector<SymTable*> imported = { this };

		public:
			int ebp_offset = 0;

			// TODO: This doesn't handle parent access (see impl notes)
			DataType& operator[](std::string key);

			bool exists(std::string key);

			// TODO: Make sure modifications of ret type work
			std::optional<DataType> get(std::string key);
			// std::optional<VarData> getVar(std::string key);
			// std::optional<SymTable> getScope(std::string key);
			// std::optional<OverloadSet> getFunc(std::string key);

			void insert(std::string key, DataType value);

			size_t size();
			size_t numVariables();

			void setParent(SymTable* p, bool offset_ebp = false);
			SymTable* getParent();
	};

}
