#pragma once

#include "cxxopts.hpp"
#include <tuple>

namespace spero {
	namespace cmd {

		cxxopts::Options getOptions();
		std::vector<std::string> parseCmdLine(cxxopts::Options&, int&, char**&);

		// Future function to parse the command line into the compilation state
		//auto parseCmd(int argc, char** argv);

		//TODO:
		// Need object to represent compile state (the command args)
		// Need function to parse commands into this state
		// Need function to parse config files (yaml) if included

	}

	// Parsing yaml files
	namespace yaml {

	}
}
