#pragma once

#include <string>

namespace spero::compiler::gen::arch {

	struct Register;

	struct Memory {
		Register& reg;
		int offset;

		Memory(Register&, int);
	};

	struct Register {
		std::string name;

		Register(std::string);

		Memory at(int=0);
	};

}