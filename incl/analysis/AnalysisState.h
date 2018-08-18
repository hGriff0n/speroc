#pragma once

#include "analysis/types.h"

namespace spero::analysis {

	struct AnalysisState {
		SymArena arena;
		AllTypes& type_list;

		inline AnalysisState(AllTypes& types) : type_list{ types } {
			arena.emplace_back(GLOBAL_SYM_INDEX);
		}
	};

}