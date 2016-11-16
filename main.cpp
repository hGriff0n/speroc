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

	// There are several problems with match

	// Adding the statement terminator doesn't help
	//"match (3, 4) { (x, y) -> 4; (y) -> 3 }";
	// Tuples can be mistaken for function calls
	//"match (3, 4) { (x, y) -> 4 (y) -> 3 }";
	// Tuple grammar doesn't work with mut
	//"match (3, 4) { mut (x, y) -> 4 y -> 3 }";
	// I have no clue for this one
	//"match x { 3 -> "{}" _ -> "X" }
	//"let x = if true do 3 else 4"
	//"if true do 3 else 4"

	// typ rule is getting called twice (from rhs_inher)

	// string_view
	// attributes
	// std::chrono

	//auto s = parser::parse(input);

	std::string input;

	while (getMultiline(std::cin, input)) {
		if (input == ":exit") break;

		// marking pretty_print as virtual causes a read access exception ???
		for (auto&& node : parser::parse(input))
			if (node) std::cout << node->pretty_print(0) << "\n";
			else std::cout << "nullptr\n";

		std::cout << std::endl;
	}

	std::cout << issues << " - Fin";
	std::cin.get();
}