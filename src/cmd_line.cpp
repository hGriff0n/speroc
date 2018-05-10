#include "interface/cmd_line.h"

#include <vector>

namespace spero::cmd {

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
			("A,allow", "Turn compilation warnings into logs", value<std::vector<std::string>>())
			("L,showlog", "Display log messages along with warnings and errors")
			("target", "Set the compilation target", value<std::string>()->default_value("win10"))
			("o,out", "Specify output file", value<std::string>()->default_value("out.exe"));


		return std::move(opts);
	}

	compiler::OptionState<cxxopts::ParseResult> parse(cxxopts::Options& opts, int& argc, char**& argv) {
		// Automatically add in "interactive" and "nodel" flags for debug running
		if (argc == 1) {
			argc = 3;
			argv = new char*[3]{ "speroc.exe", "-i", "--nodel" };
		}

		// Use the cxxopts library to parse out basic interfaces
		auto res = opts.parse(argc, argv);

		// Construct the compilation state
		compiler::OptionState<decltype(res)> state{ argv + 1, argv + argc, std::move(res) };
		argc = 1;

		// init state

		return state;
	}

}
