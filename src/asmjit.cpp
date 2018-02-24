#include "util/asmjit.h"

namespace spero::compiler::gen {

	Assembler::Assembler(Assembler&& o) noexcept : asmjit::x86::Builder{ std::move(o) }, front{ o.front }, holder{ std::move(o.holder) } {
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
		add(asmjit::x86::esp, nBytes);
	}

	void Assembler::popWords(size_t nWords) {
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
		//ffi.setAllDirty();
		emitProlog(ffi);

		// Tear-down the function
		setCursor(cursor);
		//popWords(1);
		// TODO: Clean up whatever stack allocations I used
		emitEpilog(ffi);

		// NOTE: The assembly error happens in here
		finalize();
	}
}
