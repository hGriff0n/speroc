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
		SymTable* parent = nullptr;
		int ebp_offset = 0;

		public:
			SymTable();
			~SymTable();

			int insert(std::string, int);
			std::optional<int> getVar(std::string, bool=false);

			size_t getCount();

			void setParent(SymTable*, bool=false);
			SymTable* getParent();
	};

}
