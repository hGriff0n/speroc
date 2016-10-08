
#include "parser/control.h"

//#define CATCH_CONFIG_MAIN
#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_RUNNER
#endif

#include "catch/include/catch.hpp"

#ifndef CATCH_CONFIG_MAIN

#include <iostream>
#include <string>
#include "pegtl/pegtl/analyze.hh"

const size_t issues = pegtl::analyze<spero::parser::grammar::program>();

int main(int argc, const char* argv[]) {
	std::cout << "Finished ";

	auto input = std::string{ "3 + 4" };
	auto source = std::string{ "me" };
	#define ARGS

	using namespace spero::parser;
	std::cout << pegtl::parse_string<grammar::program, actions::action>(input, source ARGS);
	std::cin.get();
}

#endif