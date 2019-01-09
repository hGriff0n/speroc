#pragma once

#include "analysis/types.h"

namespace spero::analysis {

	struct AnalysisState {
		SymArena arena;
		AllTypes type_list;

		inline AnalysisState() {
			arena.emplace_back(GLOBAL_SYM_INDEX, ScopingContext::GLOBAL);
		}

		// TODO: Change to `loadModule`
		// TODO: Figure out how we'll handle unloading modules (ie. depending on scoping context/etc.)
		inline void loadModuleTypes(const AllTypes& types) {
			// TODO: Handle collisions
			// NOTE: I'll probably handle collisions from the module standpoint (ie. before this point)
			type_list.insert(types.begin(), types.end());
		}

	};

}