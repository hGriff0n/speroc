#pragma warning(disable : 4503)

#include <iostream>
#include "compiler.h"

#include "parser/ast.h"
#include "cmd_line.h"
#include "util/utils.h"

#define ASM_COMPILER "gcc"

// Wrapper around std::getline that waits for [ENTER] to be hit twice before accepting input
template <class Stream>
Stream& getMultiline(Stream&, std::string&);


/*
 * Run the compiler and it's command-line interface
 *
 * Currently doubles as an interactive parse-tree searcher if
 *   no args or a '-i' flag is passed to evaluation
 */
int main(int argc, char* argv[]) {
	using namespace spero;
	using namespace spero::parser;

	// Parse the command line arguments
	auto opts = spero::cmd::getOptions();
	auto files = spero::cmd::parseCmdLine(opts, argc, argv);
	//auto [files, opts] = spero::cmd::parse();


	// Interactive mode for testing ast creation/parsing runs
	if (files.size() == 0 || opts["inter"].as<bool>()) {
		std::string input;

		while (std::cout << "> " && getMultiline(std::cin, input)) {
			if (input == ":q") break;

			bool succ;
			spero::parser::Stack res;

			if (input.substr(0, 2) == ":c") {
				if (!compile(res, input.substr(3), "out.exe")) {
					std::cout << "Compilation Failed\n";
					continue;
				}

			} else
				std::tie(succ, res) = compiler::parse(input);

			std::cout << "Parsing Succeeded: " << (succ ? "true" : "false") << '\n';
			printAST(std::cout, res);
		}

	// Compiler run
	} else {
		spero::parser::Stack res;

		return !compile(res, files[0], opts["out"].as<std::string>());
	}
}

bool spero::compile(spero::parser::Stack& stack, std::string in_file, std::string out_file) {
	using namespace spero;
	// TODO: Initialize timing and other compilation logging structures


	/*
	 * Perform parsing and initial recognization checks
	 */
	bool succ;
	std::tie(succ, stack) = compiler::parseFile(in_file);

	// TODO: Log report of the parsing phase


	/*
	 * Run through the analysis stages
	 */
	auto ir = compiler::analyze(stack);

	// TODO: Log report of analysis phases

	
	/*
	 * Emit final codegen processes
	 *   Note: If control reaches here, compilation should not fail
	 */
	if (succ) {
		compiler::codegen(ir, in_file, "out.s", std::cout);

		// TODO: Log report of the codegen phase

		// Forward creation of the actual executable to some other compiler
		system((ASM_COMPILER" out.s -o " + out_file).c_str());

		// TODO: Log report of the assembly phase
	}

	return succ;
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