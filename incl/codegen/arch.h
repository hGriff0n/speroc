#pragma once

#include <string>

namespace spero::compiler::gen::arch {

	struct Register;
	struct Memory;
	struct Literal;

	class Operand {
		protected:
			Operand(char);
			const char id;

		public:
			enum { REG, MEM, LIT };

			// This is very hacky, but may be necessary
			template<class Stream>
			friend Stream& operator<<(Stream& s, const Operand& op) {
				switch (op.id) {
					case Operand::REG: {
						auto& reg = reinterpret_cast<const Register&>(op);
						return s << '%' << reg.name;
					}
					case Operand::MEM: {
						auto& mem = reinterpret_cast<const Memory&>(op);
						return s << mem.offset << '(' << mem.reg << ')';
					}
					case Operand::LIT: {
						auto& lit = reinterpret_cast<const Literal&>(op);
						return s << '$' << lit.val;
					}
					default:
						return s;
				}
			}
	};

	struct Memory : Operand {
		Register& reg;
		int offset;

		Memory(Register&, int);
	};

	struct Register : Operand {
		std::string name;
		Register(std::string);

		Memory at(int=0);
	};

	struct Literal : Operand {
		int val;

		Literal(int);
	};


	// An enable_if template to ensure that operand types are valid
	//template<class Op>
	//using is_op_t = std::enable_if_t<std::is_base_of_v<Operand, Op>>;
	//template<class L, class R>
	//using if_asm_pair_t = std::enable_if_t<std::is_base_of_v<Operand, L> && std::is_base_of_v<Operand, R>>;


}