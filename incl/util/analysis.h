#pragma once

#include "enum.h"

namespace spero::analysis {
	
	/*
	 * Enum class for specifying the scoping context of the current analysis focus
	 *   Scoping context is very important in terms of variable allocation and
	 *   other semantical considerations (particularly "forward reference" uses)
	 */
	BETTER_ENUM(ScopingContext, char, GLOBAL, SCOPE, TYPE);

}