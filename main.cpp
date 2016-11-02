#pragma warning(disable : 4503)

#define CATCH_CPP14_OR_GREATER
#define CATCH_CONFIG_RUNNER

#include "catch.hpp"

#include <iostream>

#include "pegtl/analyze.hh"
#include "parser/control.h"
#include "cmd_line.h"
#include "util/utils.h"

const size_t issues = pegtl::analyze<spero::parser::grammar::program>();

int main(int argc, const char* argv[]) {
	using namespace spero;
	using namespace spero::parser;
	using namespace spero::compiler;
	//auto opts = spero::cmd::getOptions();

	auto input = "3 + 4";

	Stack s{};
	pegtl::parse_string<grammar::program, actions::action>(input, "me", s);

	for (auto it = std::begin(s); it != std::end(s); ++it)
		util::pretty_print(*it, std::cout) << "\n";

	std::cout << issues << " - Fin";
	std::cin.get();
}