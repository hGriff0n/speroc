#pragma once

#include "incl/control.h"

namespace spero::parser {
	Stack parse(std::string);
	//parse_file

	size_t num_issues();
}