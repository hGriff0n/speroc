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
			bool carrySet();
			bool paritySet();
			bool zeroSet();
			bool signSet();
			bool overflowSet();

			// Labels
			void emit(std::string&&);
			void emitLabel(std::string&&);

			// Push/pop instructions
			void push(const Register&);
			void popByte(size_t);					// Need this to work with differently sized types
			void pop(const Register&);

			// Mov instructions
			void mov(const Register&, const Register&);
			void mov(int, const Register&);
		
			// Add instructions
			void add(Memory&, Register&);
			void add(int, Register&);

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
			void setz(const Register&);					// Look into adding exception if the ZF flag isn't set
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