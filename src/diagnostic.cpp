#include "util/diagnostic.h"

namespace spero::compiler {

	Diagnostic::Diagnostic(Level lvl, std::string msg) : level{ lvl }, message{ msg } {}

	std::string Diagnostic::location() {
		if (pos) {
			auto& data = pos.value();
			std::string ret = " at line:";
			ret += std::to_string(data.line);
			ret += " col:";
			ret += std::to_string(data.byte_in_line);
			ret += " of source '";
			ret += data.source;
			return ret + '\'';
		}

		return "";
	}

	using Self = DiagnosticBuilder;

	DiagnosticBuilder::DiagnosticBuilder(Diagnostic& val) : diag{ val } {}
	
	Self& DiagnosticBuilder::setLocation(tao::pegtl::position pos) {
		diag.pos = pos;
		return *this;
	}
	
	Self& DiagnosticBuilder::setLevel(Diagnostic::Level lvl) {
		diag.level = lvl;
		return *this;
	}
}
