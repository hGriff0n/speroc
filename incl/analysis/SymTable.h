#pragma once

#include <unordered_map>
#include <optional>

#include "pegtl/position.hpp"

namespace spero::compiler::analysis {
	using Location = tao::pegtl::position;

	namespace impl {
		struct Data {
			int loc;
			std::optional<Location> src;			// TODO: Switch over to an 'id' system
			//Ast* node;

			Data();
			Data(int, Location);
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

			int insert(std::string, int, Location);
			std::optional<int> getVar(std::string, bool=false);

			size_t getCount();

			void setParent(SymTable*, bool=false);
			SymTable* getParent();
	};

}
