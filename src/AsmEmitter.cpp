#include "../incl/codegen/AsmEmitter.h"

namespace spero::compiler::gen {
	using namespace arch;

	AsmEmitter::AsmEmitter(std::ostream& s) : out{ s } {}

	bool AsmEmitter::wasCarrySet() {
		return flags[CF];
	}

	bool AsmEmitter::wasOverflowSet() {
		return flags[OF];
	}

	bool AsmEmitter::wasParitySet() {
		return flags[PF];
	}

	bool AsmEmitter::wasSignSet() {
		return flags[SF];
	}

	bool AsmEmitter::wasZeroSet() {
		return flags[ZF];
	}

	void AsmEmitter::emit(std::string&& str) {
		out << str << '\n';
	}

	void AsmEmitter::emitLabel(std::string&& label) {
		out << label << ":\n";
	}

	void AsmEmitter::push(Register& r) {
		flags.reset();
		out << "\tpush %" << r.name << '\n';
	}

	void AsmEmitter::pop(Register& r) {
		flags.reset();
		out << "\tpop %" << r.name << '\n';
	}

	void AsmEmitter::pop() {
		flags.reset();
		out << "\tadd $4, %esp\n";
	}

	void AsmEmitter::mov(Register& l, Register& r) {
		out << "\tmov %" << l.name << ", %" << r.name << '\n';

		flags.reset();
	}

	void AsmEmitter::mov(int num, Register& r) {
		out << "\tmov $" << num << ", %" << r.name << '\n';
		
		flags.reset();
	}

	void AsmEmitter::add(Memory& m, Register& r) {
		out << "\tadd " << m.offset << "(%" << m.reg.name << "), %" << r.name << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::add(int num, Register& r) {
		out << "\tadd $" << num << ", %" << r.name << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::sub(Register& r, Memory& m) {
		out << "\tsub %" << r.name << ", " << m.offset << "(%" << m.reg.name << ")\n";

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::neg(Register& r) {
		out << "\tneg %" << r.name << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::imul(Memory& m, Register& r) {
		out << "\timul " << m.offset << "(%" << m.reg.name << "), " << r.name << '\n';

		flags.reset();
		flags.set(CF);
		flags.set(OF);
	}

	void AsmEmitter::cdq() {
		out << "\tcdq\n";

		flags.reset();
	}

	void AsmEmitter::idiv(Memory& m, Register& r) {
		out << "\tidiv " << m.offset << "(%" << m.reg.name << "), " << r.name << '\n';

		flags.reset();
	}

	void AsmEmitter::_xor(Register& l, Register& r) {
		out << "\txor %" << l.name << ", %" << r.name << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::test(Register& l, Register& r) {
		out << "\ttest %" << l.name << ", %" << r.name << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::cmp(Register& l, Memory& m) {
		out << "\tcmp %" << l.name << ", " << m.offset << "(%" << m.reg.name << ")\n";

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::setz(Register& r) {
		out << "\tsetz %" << r.name << '\n';

		flags.reset();
	}

	void AsmEmitter::setl(Register& r) {
		out << "\tsetl %" << r.name << '\n';

		flags.reset();
	}

	void AsmEmitter::leave() {
		out << "\tleave\n";

		flags.reset();
	}
	
	void AsmEmitter::ret() {
		out << "\tret\n";

		flags.reset();
	}
}