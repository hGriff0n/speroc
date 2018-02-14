#pragma once

#include <ostream>
#include <bitset>
#include "_arch.h"

namespace spero::compiler::gen {
	using namespace arch;

// Macros to simplify emitter definitions
#define CPU_FLAG static constexpr size_t

	/* TODO:
     *   Adapt 'popByte' to consider differently sized types (dword v. qword)
	 *   Add in ability to match _Register and value sizes (ie. %al can't hold INT_MAX)
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
			void push(_Register&);
			void pushByte(size_t);
			void pop(_Register&);
			void popByte(size_t);

			// Mov instructions
			void mov(_Register&, _Register&);
			void mov(Literal, _Register&);
			void mov(Memory&, _Register&);
			void mov(_Register&, Memory&);

			void movzx(_Register&, _Register&);

			// Add instructions
			void add(Literal, _Register&);
			void add(Memory&, _Register&);

			// Sub instructions
			void sub(_Register&, Memory&);
			void neg(_Register&);

			// Mul instructions
			void imul(Memory&, _Register&);

			// Div instructions
			void idiv(Memory&);
			void cdq();

			// Logical instructions
			void _xor(_Register&, _Register&);
			void not(_Register&);
			void _or(Memory&, _Register&);

			// Test/Compare instructions
			void test(_Register&, _Register&);
			void cmp(_Register&, Memory&);
			void cmp(Memory&, _Register&);

			// Set instructions
			void setz(_Register&);
			void setnz(_Register&);
			void setl(_Register&);
			void setl(Memory&);
			void setle(_Register&);
			void setg(_Register&);
			void setge(_Register&);

			// Control flow
			void ret();
			void leave();
	};
}
