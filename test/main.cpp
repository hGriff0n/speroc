
/*
 * Project to hold all the testing code for the `speroc` project
 */

#define CATCH_CPP14_OR_GREATER
#define CATCH_CONFIG_MAIN

#include "catch.hpp"

//#include "parser\parser.h"

unsigned int factorial(unsigned int n) {
	return n <= 1 ? n : n * factorial(n - 1);
}

// This isn't a good testing framework, too high level
// I need to provide a more complete coverage of every system
TEST_CASE("Parsing", "[parser]") {
	SECTION("Testing Parser Recognition", "[recognition]") {

	}

	SECTION("Testing Parser Assembly", "[assembly]") {

	}

	SECTION("Testing Parser Errors", "[errors]") {

	}
}

TEST_CASE("Factorials", "[factorial]") {
	REQUIRE(factorial(1) == 1);
}