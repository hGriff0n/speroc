#pragma warning(disable : 4503)

#define CATCH_CPP14_OR_GREATER
#define CATCH_CONFIG_RUNNER

#include "catch.hpp"

#include <iostream>
#include "parser\parser.h"

#include "cmd_line.h"
#include "util/utils.h"

const size_t issues = spero::parser::num_issues();

// Wrapper around std::getline that waits for [ENTER] to be hit twice before accepting input (to simplify multiline repl testing)
template <class Stream>
Stream& getMultiline(Stream& in, std::string& s) {
	std::getline(in, s);
	if (s == ":exit") return in;

	std::string tmp{};
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

	/*
	TODO: Current grammar problems
		"match (3, 4) { (x, y) -> 4; (y) -> 3 }"
		"match (3, 4) { x, y -> 4 (y) -> 3 }"
		"match (3, 4) { mut (x, y) -> 4 y -> 3 }"
		"match x { 3 -> "{}" _ -> "X" }"
		"let x = if true do 3 else 4"
		"if true do 3 else 4"
		"def foo = (x) -> x(3)"
		"def foo = (x) -> 3.x"
	*/

	// The dot-control actions don't work for function definitions
		// They will attempt to work with values that aren't there

	/*
	TODO: Features to look into using
		string_view
		attributes
		std::chrono
	 */

	std::string input;

	while (getMultiline(std::cin, input)) {
		if (input == ":exit") break;

		// marking pretty_print as virtual causes a read access exception ???
		auto res = parser::parse(input);
		//for (auto&& node : parser::parse(input))
		for (auto&& node : res)
			if (node) std::cout << node->pretty_print(0) << "\n";
			else std::cout << "nullptr\n";

		std::cout << std::endl;
	}

	std::cout << issues << " - Fin";
	std::cin.get();
}