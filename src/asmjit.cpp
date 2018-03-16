#include "util/asmjit.h"

namespace spero::compiler::gen {

	Assembler::Assembler(Assembler&& o) noexcept : asmjit::x86::Builder{ std::move(o) }, front{ o.front }, holder{ std::move(o.holder) }, allocated_stack{ o.allocated_stack } {
		_code = &holder;

		if (auto idx = holder._emitters.indexOf(&o); idx != asmjit::Globals::kNotFound) {
			holder._emitters[idx] = this;
		}

		o.front = nullptr;
	}

	Assembler::~Assembler() noexcept {
		holder.detach(this);
	}

	void Assembler::popBytes(size_t nBytes) {
		if (nBytes != 0) {
			add(asmjit::x86::esp, nBytes);
		}
	}

	void Assembler::popWords(size_t nWords) {
		// Quick and dirty hack to get compilation working
		// I know this technically isn't a word, but a dword
		popBytes(nWords * 4);
	}

	asmjit::CodeHolder* Assembler::get() {
		return &holder;
	}

	// Setup the assembler for interpretation
	// Basically turns the produced assembly into a `int()` function
	void Assembler::makeIFunction() {
		// Setup the function
		auto* cursor = setCursor(front);
		asmjit::FuncDetail func;
		func.init(asmjit::FuncSignatureT<int>(asmjit::CallConv::kIdHost));

		asmjit::FuncFrame ffi;
		ffi.init(func);
		ffi.setAllDirty();
		emitProlog(ffi);
		mov(asmjit::x86::ebp, asmjit::x86::esp);

		// Tear-down the function
		setCursor(cursor);
		popBytes(allocated_stack);
		emitEpilog(ffi);

		finalize();
	}

	void Assembler::setAllocatedStack(size_t size) {
		allocated_stack = size;
	}
}
