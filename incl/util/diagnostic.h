#pragma once

#include <string>
#include <optional>
#include "pegtl/position.hpp"

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
		std::optional<tao::pegtl::position> pos;
		
		Diagnostic(Level, std::string);

		std::string location();
	};


	/*
	 * Small structure to enable the 'builder' pattern for constructing
	 *   more complex error messages, particularly with source attaches
	 */
	class DiagnosticBuilder {
		Diagnostic& diag;
		using Self = DiagnosticBuilder;

		public:
			DiagnosticBuilder(Diagnostic&);

			Self& setLocation(tao::pegtl::position);
			Self& setLevel(Diagnostic::Level);
	};
}