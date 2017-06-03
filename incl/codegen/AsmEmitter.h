#pragma once

#include <ostream>
#include <bitset>
#include "arch.h"

namespace spero::compiler::gen {
	using namespace arch;

// Macros to simplify emitter definitions
#define CPU_FLAG static constexpr size_t

	/* TODO:
     *   Adapt 'popByte' to consider differently sized types (dword v. qword)
	 *   Add in ability to match register and value sizes (ie. %al can't hold INT_MAX)
	 */

	//[[ maybe_unused ]]
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
			bool carrySet();
			bool paritySet();
			bool zeroSet();
			bool signSet();
			bool overflowSet();

			// Labels
			void write(std::string&&);
			void label(std::string&&);

			// Push/pop instructions
			void push(Register&);
			void pushByte(size_t);
			void pop(Register&);
			void popByte(size_t);

			// Mov instructions
			void mov(Register&, Register&);
			void mov(Literal, Register&);
			void mov(Memory&, Register&);
			void mov(Register&, Memory&);
		
			// Add instructions
			void add(Literal, Register&);
			void add(Memory&, Register&);

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
			void not(Register&);
			void _or(Memory&, Register&);

			// Test/Compare instructions
			void test(Register&, Register&);
			void cmp(Register&, Memory&);
			void cmp(Memory&, Register&);

			// Set instructions
			void setz(Register&);
			void setnz(Register&);
			void setl(Register&);
			void setl(Memory&);
			void setg(Register&);
			void setle(Register&);

			// Control flow
			void ret();
			void leave();
	};
}