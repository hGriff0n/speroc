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

		std::vector<std::string> parseCmdLine(cxxopts::Options& opts, int& argc, char**& argv) {
			opts.parse(argc, argv);
			
			auto tmp = argc;
			argc = 1;

			return { argv + 1, argv + tmp };
		}

	}
}
