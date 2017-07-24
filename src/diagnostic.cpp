#include "util/diagnostic.h"

namespace spero::compiler {

	Diagnostic::Diagnostic(Level lvl, std::string msg) : level{ lvl }, message{ msg } {}
}