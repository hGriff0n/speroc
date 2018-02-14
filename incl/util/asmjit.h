#pragma once

#define ASMJIT_DISABLE_COMPILER
#undef ASMJIT_EMBED
#define ASMJIT_BUILD_EMBED
#include <asmjit\asmjit.h>

// Provides a more uniform interface for usage of asmjit structures
// For instance, the x86 namespace defines Reg and Mem structs, but
// Doesn't define the Imm or Label structs. While this has its reasons
// I feel it is slightly odd and confusing for normal development.

namespace asmjit {
	namespace x86 {
		using Imm = ::asmjit::Imm;
		using Label = ::asmjit::Label;
	}
}
