#pragma warning(disable : 4503)

#define CATCH_CPP14_OR_GREATER
#define CATCH_CONFIG_RUNNER

#include "catch.hpp"

#include <iostream>
#include "parser\parser.h"

#include "cmd_line.h"
#include "util/utils.h"

const size_t issues = spero::parser::num_issues();

int main(int argc, const char* argv[]) {
	using namespace spero;
	using namespace spero::parser;
	//auto opts = spero::cmd::getOptions();

	// This isn't working
	auto input = "([3, true],  { 4 }, \"Hello\")";


	// string_view
	// attributes
	// std::chrono

	//auto s = parser::parse(input);

	// marking pretty_print as virtual causes a read access exception ???
	for (auto&& node : parser::parse(input))
		if (node) std::cout << node->pretty_print(0) << "\n";
		else std::cout << "nullptr\n";

	std::cout << issues << " - Fin";
	std::cin.get();
}