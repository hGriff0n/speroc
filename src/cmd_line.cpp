#include "cmd_line.h"
#include <iostream>

namespace spero {
	namespace cmd {

		cxxopts::Options getOptions() {
			using namespace cxxopts;

			auto opts = Options("speroc");

			opts.add_options()
				("i,inter", "Enter interactive mode")
				("d,debug", "Enable debugging")
				("o,out", "Specify output file", value<std::string>()->default_value("out.exe"));

			return std::move(opts);
		}

		spero::compiler::OptionState<cxxopts::Options> parse(int& argc, char**& argv) {
			// Use the cxxopts library to parse out basic interfaces
			auto opts = getOptions();
			opts.parse(argc, argv);

			// Construct the compilation state
			compiler::OptionState<cxxopts::Options> state{ argv + 1, argv + argc, std::move(opts) };
			argc = 1;

			// init state

			return state;
		}

	}
}
