#include "cmd_line.h"

namespace spero {
	namespace cmd {

		cxxopts::Options&& getOptions() {
			using namespace cxxopts;

			auto opts = Options("speroc");

			opts.add_options()
				("d, debug", "Enable debugging")
				("o,out", "Specify output file", value<std::string>());

			return std::move(opts);
		}

	}
}
