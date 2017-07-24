#pragma once

#include <string>

namespace spero::compiler {
	
	/*
	 * A small structure enabling the wrapping of error statistics
	 * The design also allows for it's usage in log and warning contexts
	 */
	struct Diagnostic {
		/*
		* Enum class specifying the level of concern that the
		* diagnostic instance should be taken to represent
		*/
		enum class Level {
			LOG,
			WARNING,
			ERROR
		} level;

		std::string message;
		
		Diagnostic(Level, std::string);
	};
}