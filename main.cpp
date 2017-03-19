#pragma warning(disable : 4503)

#include <iostream>
#include "parser.h"
#include "compiler.h"

#include "cmd_line.h"
#include "util/utils.h"

// Wrapper around std::getline that waits for [ENTER] to be hit twice before accepting input
template <class Stream>
Stream& getMultiline(Stream&, std::string&);

// Helper function to print out the ast
template<class Stream>
Stream& writeAST(Stream&, const spero::parser::Stack&);

void compile(spero::parser::Stack&, std::string, std::string);

int main(int argc, const char* argv[]) {
	using namespace spero;
	using namespace spero::parser;
	//auto opts = spero::cmd::getOptions();
	//auto [files, opts] = spero::cmd::parse();

	/*
	TODO: Features to look into using
		string_view
		attributes
		std::chrono
	 */

	// Quick and simple command line interface for testing
	if (argc > 1) {
		std::string out = (argc > 2) ? argv[2] : "spero.exe";
		std::string gcc = "gcc out.s -o " + out;

		bool succ;
		spero::parser::Stack res;

		std::tie(succ, res) = parser::parseFile(argv[1]);
		if (succ) {
			compiler::compile(res, argv[1], "out.s", std::cout);
			system(gcc.c_str());

		} else {
			std::cout << "Parsing failed\n";
		}

		return 0;
	}

	std::string input;

	while (std::cout << "> " && getMultiline(std::cin, input)) {
		if (input == ":q") break;

		bool succ;
		spero::parser::Stack res;

		if (input.substr(0, 2) == ":c") {
			auto file = input.substr(3);
			std::tie(succ, res) = parser::parseFile(file);

			if (succ) {
				compile(res, file, "out.s");
				system("gcc out.s -o spero.exe");
			}
			//res = parser::parse(findFile(input.substr(3), "spr", "spqr"));

		} else {
			std::tie(succ, res) = parser::parse(input);
		}

		std::cout << "Parsing Succeeded: " << (succ ? "true" : "false") << '\n';
		writeAST(std::cout, res);
	}

	//std::cout << issues << " - Fin";
	//std::cin.get();
}

template <class Stream>
Stream& getMultiline(Stream& in, std::string& s) {
	std::getline(in, s);
	if (s == ":q") return in;

	std::string tmp;
	while (std::getline(in, tmp)) {
		if (tmp == "") return in;

		s += "\n" + tmp;
	}

	return in;
}


template<class Stream>
Stream& writeAST(Stream& s, const spero::parser::Stack& stack) {
	for (const auto& node : stack)
		if (node) node->prettyPrint(s, 0) << '\n';
		else s << "nullptr\n";

	return s << '\n';
}


/*
 * Codegen section 
 *
 * TODO: Move into a seperate file
 */

#include <fstream>
#include "codegen/AsmGenerator.h"
void compile(spero::parser::Stack& s, std::string in, std::string out) {
	using namespace spero::parser;
	std::cout << "Starting compilation phase...\n";

	// Open the output file
	std::ofstream o{ out };
	std::cout << "Opened file " << out << " for compilation output\n";

	// Output file header information
	o << "\t.file \"" << in << "\"\n.text\n";

	auto visitor = spero::compiler::codegen::AsmGenerator{ o };

	// Print everything directly to the file
	for (const auto& node : s)
		node->visit(visitor);

	o << '\n';

	// End the compilation phase
	std::cout << "Ending compilation phase...\n";
}

// TODO: Have Module Declarations collect everything as a single string