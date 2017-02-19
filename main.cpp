#pragma warning(disable : 4503)

#include <iostream>
#include "parser/parser.h"

#include "cmd_line.h"
#include "util/utils.h"

// Big performance hit
//const size_t issues = spero::parser::num_issues();

// Wrapper around std::getline that waits for [ENTER] to be hit twice before accepting input (to simplify multiline repl testing)
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

	std::string input;

	while (getMultiline(std::cin, input)) {
		if (input == ":q") break;

		bool succ;
		spero::parser::Stack res;

		if (input.substr(0, 2) == ":l") {
			std::tie(succ, res) = parser::parseFile(input.substr(3));
			//res = parser::parse(findFile(input.substr(3), "spr", "spqr"));

		} else {
			std::tie(succ, res) = parser::parse(input);
		}

		std::cout << "Parsing Succeeded: " << (succ ? "true" : "false") << '\n';
		for (const auto& node : res)
			if (node) node->prettyPrint(std::cout, 0) << "\n";
			else std::cout << "nullptr\n";

		std::cout << std::endl;
	}

	//std::cout << issues << " - Fin";
	//std::cin.get();
}