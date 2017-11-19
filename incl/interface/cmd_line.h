#pragma once

#include "cxxopts.hpp"
#include <tuple>

#include "CompilationState.h"

namespace spero {
	namespace cmd {
		// Parse the entire command line into the default compilation state
		compiler::OptionState<cxxopts::Options> parse(int&, char**&);

		//TODO:
		// Need object to represent compile state (the command args)
		// Need function to parse commands into this state
		// Need function to parse config files (yaml) if included

	}


	// Parsing yaml files
	namespace yaml { }
}
