#pragma once

#include <unordered_map>
#include <optional>

namespace spero::compiler::analysis {
	namespace impl {

		struct Data {
			int loc;
			//Ast* node;
		};

	}

	/*
	 * Symbol Table class
	 */
	class SymTable {
		std::unordered_map<std::string, impl::Data> var_data;
		SymTable* parent;

		public:
			SymTable();
			SymTable(SymTable*);
			~SymTable();

			void insert(std::string, int);
			std::optional<int> getVar(std::string);

			size_t getCount();
	};

}
