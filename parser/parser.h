#pragma once

#include "incl/ast/all.h"

namespace spero::parser {
	std::tuple<bool, Stack> parse(std::string);
	std::tuple<bool, Stack> parseFile(std::string);
	//parse_file

	size_t num_issues();
}