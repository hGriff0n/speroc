#include "../incl/codegen/arch.h"

namespace spero::compiler::gen::arch {

	// Memory methods
	Memory::Memory(Register& r, int off) : reg{ r }, offset{ off } {}


	// Register methods
	Register::Register(std::string n) : name{ n } {}

	Memory Register::at(int off) {
		return Memory{ *this, off };
	}

}