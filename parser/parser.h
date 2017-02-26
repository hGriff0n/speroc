#pragma once

#include "incl/ast/ast.h"
#include "incl/AsmGenerator.h"

namespace spero::parser {
	std::tuple<bool, Stack> parse(std::string);
	std::tuple<bool, Stack> parseFile(std::string);
	//parse_file

	size_t num_issues();
}