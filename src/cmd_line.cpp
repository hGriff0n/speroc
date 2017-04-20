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

		spero::compiler::CompilationState parse(int& argc, char**& argv) {
			// Parse out the command line
			auto opts = getOptions();
			opts.parse(argc, argv);

			return parse(opts, argc, argv);
		}

		spero::compiler::CompilationState parse(cxxopts::Options& opts, int& argc, char**& argv) {
			// Construct the compilation state from the parsed options
			compiler::CompilationState state{ argv + 1, argv + argc };
			argc = 1;

			// init state

			return state;
		}

	}
}
