#include "../incl/codegen/_arch.h"

namespace spero::compiler::gen::arch {

	// Operand constructor
	Operand::Operand(char c) : id{ c } {}

	// Memory methods
	Memory::Memory(_Register& r, int off) : Operand{ Operand::MEM }, reg { r }, offset{ off } {}


	// Register methods
	_Register::_Register(std::string n) : Operand{ Operand::REG }, name { n } {}

	Memory _Register::at(int off) {
		return Memory{ *this, off };
	}

	// Literal methods
	Literal::Literal(int i) : Operand{ Operand::LIT }, val { i } {}

}
