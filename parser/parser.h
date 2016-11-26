#pragma once

#include "incl/ast/all.h"

namespace spero::parser {
	Stack parse(std::string);
	Stack parseFile(std::string);
	//parse_file

	size_t num_issues();
}