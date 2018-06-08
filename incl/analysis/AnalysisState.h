#pragma once

#include "analysis/types.h"

namespace spero::analysis {

	struct AnalysisState {
		std::unique_ptr<SymTable> table;
		AllTypes& type_list;

		inline AnalysisState(AllTypes& types) : AnalysisState{ std::make_unique<SymTable>(), types } {}
		inline AnalysisState(std::unique_ptr<SymTable> table, AllTypes& types) : table{ std::move(table) }, type_list{ types } {}
	};

}