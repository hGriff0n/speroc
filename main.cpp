

#define CATCH_CPP14_OR_GREATER
#define CATCH_CONFIG_RUNNER

#include "catch.hpp"

#include "pegtl/analyze.hh"
#include "parser/control.h"
#include "cmd_line.h"
#include "util/utils.h"

#include <iostream>

const size_t issues = pegtl::analyze<spero::parser::grammar::program>();

int main(int argc, const char* argv[]) {
	using namespace spero;
	using namespace spero::parser;
	//auto opts = spero::cmd::getOptions();

	auto input = "true";

	Stack s{};
	pegtl::parse_string<grammar::program, actions::action>(input, "me", s);

	for (auto elem : s)
		util::pretty_print(elem, std::cout);

	std::cout << "Fin";
	std::cin.get();
}
