#pragma once

#include "parser.h"
#include "codegen/AsmGenerator.h"

#include <fstream>

namespace spero::compiler {

	void compile(spero::parser::Stack& s, std::string in, std::string out, std::ostream& debug) {
		using namespace spero::parser;
		debug << "Starting compilation phase...\n";

		// Open the output file
		std::ofstream o{ out };
		debug << "Opened file " << out << " for compilation output\n";

		// Output file header information
		o << "\t.file \"" << in << "\"\n.text\n";

		auto visitor = spero::compiler::codegen::AsmGenerator{ o };

		// Print everything directly to the file
		for (const auto& node : s)
			node->visit(visitor);

		o << '\n';

		// End the compilation phase
		debug << "Ending compilation phase...\n";
	}

}