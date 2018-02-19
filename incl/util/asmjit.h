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
		asmjit::CBNode* front;

		public:
			inline Assembler() : asmjit::x86::Builder{ nullptr }, front{ cursor() } {
				holder.init(asmjit::CodeInfo{ asmjit::ArchInfo::kIdX64 });
				holder.attach(this);
			}
			inline Assembler(Assembler&& o) : asmjit::x86::Builder{ std::move(o) }, front{ o.front }, holder{ std::move(o.holder) } {
				auto idx = holder._emitters.indexOf(&o);
				_code = &holder;
				if (idx != asmjit::Globals::kNotFound) {
					holder._emitters[idx] = this;
				}

				o.front = nullptr;
			}
			virtual ~Assembler() noexcept {
				holder.detach(this);
			}

			void popBytes(size_t nBytes) {
				add(asmjit::x86::esp, nBytes);
			}

			void popWords(size_t nWords) {
				popBytes(nWords * 4);
			}

			asmjit::CodeHolder* get() {
				return &holder;
			}

			// Setup the assembler for interpretation
			// Basically turns the produced assembly into a `int()` function
			void makeIFunction() {
				// Setup the function
				auto* cursor = setCursor(front);
				asmjit::FuncDetail func;
				func.init(asmjit::FuncSignatureT<int>(asmjit::CallConv::kIdHost));

				asmjit::FuncFrame ffi;
				ffi.init(func);
				ffi.setAllDirty();
				emitProlog(ffi);

				// Tear-down the function
				setCursor(cursor);
				emitEpilog(ffi);

				finalize();
			}

			using Function = int(*)();
	};
}
