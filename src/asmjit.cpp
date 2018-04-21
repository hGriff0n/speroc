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

	void Assembler::popBytes(size_t num_bytes) {
		if (num_bytes != 0) {
			add(asmjit::x86::esp, num_bytes);
		}
	}

	void Assembler::popWords(size_t num_words) {
		// Quick and dirty hack to get compilation working
		// I know this technically isn't a word, but a dword
		popBytes(num_words * 4);
	}

	void Assembler::pushBytes(size_t num_bytes) {
		if (num_bytes != 0) {
			sub(asmjit::x86::esp, num_bytes);
		}
	}

	void Assembler::pushWords(size_t num_words) {
		// Quick and dirty hack to get compilation working
		// I know this technically isn't a word, but a dword
		pushBytes(num_words * 4);
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
