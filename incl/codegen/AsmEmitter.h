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
			void push(const Register&);
			void popByte(size_t);
			void pop(const Register&);

			// Mov instructions
			void mov(const Register&, const Register&);
			void mov(Literal, const Register&);
		
			// Add instructions
			void add(Literal, const Register&);
			void add(const Memory&, const Register&);

			// Sub instructions
			void sub(const Register&, const Memory&);
			void neg(const Register&);

			// Mul instructions
			void imul(const Memory&, const Register&);
			
			// Div instructions
			void idiv(const Memory&, const Register&);
			void cdq();

			// Logical instructions
			void _xor(const Register&, const Register&);
			void not(const Register&);
			void _or(const Memory&, const Register&);

			// Test/Compare instructions
			void test(const Register&, const Register&);
			void cmp(const Register&, const Memory&);
			void cmp(const Memory&, const Register&);

			// Set instructions
			void setz(const Register&);
			void setnz(const Register&);
			void setl(const Register&);
			void setl(const Memory&);
			void setg(const Register&);
			void setle(const Register&);

			// Control flow
			void ret();
			void leave();
	};
}