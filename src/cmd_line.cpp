#include "cmd_line.h"
#include <iostream>
#include <vector>

namespace spero {
	namespace cmd {

		cxxopts::Options getOptions() {
			using namespace cxxopts;

			auto opts = Options("speroc");

			// TODO: Need to turn -W into a 'vector' type field
			opts.add_options()
				("i,interactive", "Enter interactive mode")
				("g,debug", "Enable addition of debugging information to created binaries")
				("nodel", "Prevent deletion of temporary files during compilation")
				("v,verbose", "Turn on verbose reporting of the compilation progress")
				("C,stop", "Stop compilation at the assembly source file")
				("diagnostics", "Specify a file to send diagnostic information to", value<std::string>())
				("emit", "List of temporary files that the compiler should not delete", value<std::string>())
				("W,warn", "Turn compilation warnings into errors", value<std::vector<std::string>>())
				("L,showlog", "Display log messages along with warnings and errors")
				("target", "Set the compilation target", value<std::string>()->default_value("win10"))
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
