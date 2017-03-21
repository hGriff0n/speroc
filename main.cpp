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

int main(int argc, char* argv[]) {
	using namespace spero;
	using namespace spero::parser;

	// Parse the command line arguments
	auto opts = spero::cmd::getOptions();
	auto files = spero::cmd::parseCmdLine(opts, argc, argv);
	//auto [files, opts] = spero::cmd::parse();

	/*
	TODO: Features to look into using
		string_view
		attributes
		std::chrono
	 */

	// Interactive mode for testing ast creation/parsing runs
	if (files.size() == 0 || opts["inter"].as<bool>()) {
		std::string input;

		while (std::cout << "> " && getMultiline(std::cin, input)) {
			if (input == ":q") break;

			bool succ;
			spero::parser::Stack res;

			if (input.substr(0, 2) == ":c") {
				auto file = input.substr(3);
				std::tie(succ, res) = parser::parseFile(file);

				if (succ) {
					compiler::compile(res, file, "out.s", std::cout);
					system("gcc out.s -o out.exe");
				}
				//res = parser::parse(findFile(input.substr(3), "spr", "spqr"));

			} else {
				std::tie(succ, res) = parser::parse(input);
			}

			std::cout << "Parsing Succeeded: " << (succ ? "true" : "false") << '\n';
			writeAST(std::cout, res);
		}

	// Compiler run
	} else {
		std::string gcc_cmd = "gcc out.s -o " + opts["out"].as<std::string>();

		bool succ;
		spero::parser::Stack res;

		std::tie(succ, res) = parser::parseFile(files[0]);
		if (succ) {
			compiler::compile(res, files[0], "out.s", std::cout);
			system(gcc_cmd.c_str());

		} else
			std::cout << "Parsing failed\n";
	}
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