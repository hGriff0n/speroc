#pragma once

#include "grammar.h"

namespace spero {
	namespace parser {
		namespace actions {
			template<class Rule>
			struct action : pegtl::nothing<Rule> {};
		}
	}
}