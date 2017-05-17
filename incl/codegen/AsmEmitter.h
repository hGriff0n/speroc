#pragma once

#include <ostream>
#include <bitset>
#include "arch.h"

namespace spero::compiler::gen {
	using namespace arch;

#define CPU_FLAG static constexpr size_t

	class AsmEmitter {
		std::ostream& out;
		std::bitset<5> flags;
		
		CPU_FLAG CF = 0;
		CPU_FLAG PF = 1;
		CPU_FLAG ZF = 2;
		CPU_FLAG SF = 3;
		CPU_FLAG OF = 4;
		static constexpr size_t ALL_FLAGS = 31;

		public:
			AsmEmitter(std::ostream&);

			// Query whether the last instruction may have set one of the flags
			bool wasCarrySet();
			bool wasParitySet();
			bool wasZeroSet();
			bool wasSignSet();
			bool wasOverflowSet();

			// Labels
			void emit(std::string&&);
			void emitLabel(std::string&&);

			// Push/pop instructions
			void push(Register&);
			void pop();
			void pop(Register&);

			// Mov instructions
			void mov(Register&, Register&);
			void mov(int, Register&);
		
			// Add instructions
			void add(Memory&, Register&);
			void add(int, Register&);

			// Sub instructions
			void sub(Register&, Memory&);
			void neg(Register&);

			// Mul instructions
			void imul(Memory&, Register&);
			
			// Div instructions
			void idiv(Memory&, Register&);
			void cdq();

			// Logical instructions
			void _xor(Register&, Register&);

			// Test/Compare instructions
			void test(Register&, Register&);
			void cmp(Register&, Memory&);

			// Set instructions
			void setz(Register&);
			void setl(Register&);

			// Control flow
			void ret();
			void leave();
	};
}