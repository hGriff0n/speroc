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
	/*
	 * Provides a wrapper class around the asmjit Builder and CodeHolder structs
	 * In order to facilitate value semantics and code "movement".
	 *
	 * Basically, to "return" a Builder from a function, we also have to return
	 * It's attached CodeHolder. Since both classes have copying disabled, and
	 * The attaching is done through pointer semantics, they must be moved instead
	 * Instead of copied and require some pointer rebalancing after the fact.
	 */
	class Assembler : public asmjit::x86::Builder {
		asmjit::CodeHolder holder;
		asmjit::CBNode* front;
		size_t allocated_stack = 0;

		public:
			inline Assembler() noexcept : asmjit::x86::Builder{ nullptr }, front{ cursor() } {
				// TODO: Customize the code info for the architecture I'm producing code for (but then I kinda need to get away from x86)
				asmjit::CodeInfo info;
				info._archInfo = asmjit::CpuInfo::host().archInfo();
				info._stackAlignment = uint8_t(16);
				info._cdeclCallConv = asmjit::CallConv::kIdHostCDecl;
				info._stdCallConv = asmjit::CallConv::kIdHostStdCall;
				info._fastCallConv = asmjit::CallConv::kIdHostFastCall;
				holder.init(std::move(info));

				holder.attach(this);
			}
			Assembler(Assembler&& o) noexcept;
			virtual ~Assembler() noexcept;

			// Helper functions for common use cases
			void popBytes(size_t num_bytes);
			void popWords(size_t num_words);
			void pushBytes(size_t num_bytes);
			void pushWords(size_t num_words);

			// Retrieve the code holder for interpretation
			asmjit::CodeHolder* get();

			// Setup the assembler for interpretation
			// Basically turns the produced assembly into a `int()` function
			void makeIFunction();

			void setAllocatedStack(size_t size);

			using Function = int(*)();
	};
}
