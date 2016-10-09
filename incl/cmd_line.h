#pragma once

#include "cxxopts.hpp"

namespace spero {
	namespace cmd {

		cxxopts::Options&& getOptions();
		//auto parseCmd(...);

		//TODO:
		// Need object to represent compile state (the command args)
		// Need function to parse commands into this state
		// Need function to parse config files (yaml) if included

	}
}