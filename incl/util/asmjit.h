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

namespace spero::compiler::gen {

	// TODO: The name could use some slight tweaking
	class Assembler : public asmjit::x86::Builder {
		asmjit::CodeHolder holder;

		public:
			inline Assembler() : asmjit::x86::Builder{ nullptr } {
				asmjit::CodeInfo arch;
				arch.init(asmjit::ArchInfo::kIdX86);

				holder.init(arch);
				holder.attach(this);
			}
			inline Assembler(Assembler&& o) : asmjit::x86::Builder{ std::move(o) }, holder{ std::move(o.holder) } {
				auto idx = holder._emitters.indexOf(&o);
				_code = &holder;
				if (idx != asmjit::Globals::kNotFound) {
					holder._emitters[idx] = this;
				}
			}
			virtual ~Assembler() noexcept {
				holder.detach(this);
			}
	};
}
