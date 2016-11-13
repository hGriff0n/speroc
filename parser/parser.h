#pragma once

#include "incl/ast.h"

namespace spero::parser {
	Stack parse(std::string);
	//parse_file

	size_t num_issues();
}