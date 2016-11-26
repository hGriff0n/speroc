#pragma once

#include "actions.h"

namespace spero::parser::control {
	template<class Rule>
	struct control : pegtl::normal<Rule> {};
}