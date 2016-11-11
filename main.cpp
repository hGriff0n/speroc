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
	(void)argc;
	(void)argv;

	using namespace spero;
	using namespace spero::parser;
	//auto opts = spero::cmd::getOptions();

	auto input = "mod std:util:Array";

	auto s = parser::parse(input);

	for (auto it = std::begin(s); it != std::end(s); ++it);
		//util::pretty_print(*it, std::cout) << "\n";

	std::cout << issues << " - Fin";
	std::cin.get();
}