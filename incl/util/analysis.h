#pragma once

#include "enum.h"

namespace spero::analysis {
	
	/*
	 * Enum class for specifying the scoping context of the current analysis focus
	 *   Scoping context is very important in terms of variable allocation and
	 *   other semantical considerations (particularly "forward reference" uses)
	 */
	BETTER_ENUM(ScopingContext, char, GLOBAL, SCOPE, TYPE)


	/*
	 * Variant classes for specifying how variable data is stored and accessed
	 */
	namespace memory {
		struct Stack {
			int ebp_offset;
		};
		struct Member {
			int base_offset;
		};
		struct Global {
			int location;
			bool is_static;
		};
		// Primarily for reasoning about function arguments (which may be passed in register)
		struct Register {
			// TODO: Figure out a way to specify which register
		};

		using Locations = std::variant<Stack, Member, Global, Register>;
	}

}