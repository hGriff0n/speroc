#include "../incl/codegen/AsmEmitter.h"

namespace spero::compiler::gen {
	using namespace arch;

	AsmEmitter::AsmEmitter(std::ostream& s) : out{ s } {}

	bool AsmEmitter::carrySet() {
		return flags[CF];
	}

	bool AsmEmitter::overflowSet() {
		return flags[OF];
	}

	bool AsmEmitter::paritySet() {
		return flags[PF];
	}

	bool AsmEmitter::signSet() {
		return flags[SF];
	}

	bool AsmEmitter::zeroSet() {
		return flags[ZF];
	}

	void AsmEmitter::write(std::string&& str) {
		out << str << '\n';
	}

	void AsmEmitter::label(std::string&& label) {
		out << label << ":\n";
	}

	void AsmEmitter::push(Register& r) {
		out << "\tpush " << r << '\n';
		
		flags.reset();
	}

	void AsmEmitter::pushByte(size_t n) {
		Register esp{ "esp" };
		out << "\tadd $" << (n * 4) << ", " << esp << '\n';

		flags.reset();
	}

	void AsmEmitter::pop(Register& r) {
		out << "\tpop " << r << '\n';
		
		flags.reset();
	}

	void AsmEmitter::popByte(size_t n) {
		Register esp{ "esp" };
		out << "\tadd $" << (n * 4) << ", " << esp << '\n';

		flags.reset();
	}

	void AsmEmitter::mov(Register& src, Register& dest) {
		out << "\tmov " << src << ", " << dest << '\n';

		flags.reset();
	}

	void AsmEmitter::mov(Literal num, Register& dest) {
		out << "\tmov " << num << ", " << dest << '\n';
		
		flags.reset();
	}

	void AsmEmitter::mov(Memory& src, Register& dest) {
		out << "\tmov " << src << ", " << dest << '\n';

		flags.reset();
	}

	void AsmEmitter::mov(Register& src, Memory& dest) {
		out << "\tmov " << src << ", " << dest << '\n';

		flags.reset();
	}

	void AsmEmitter::add(Memory& src, Register& dest) {
		out << "\tadd " << src << ", " << dest << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::add(Literal num, Register& dest) {
		out << "\tadd " << num << ", " << dest << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::sub(Register& src, Memory& dest) {
		out << "\tsub " << src << ", " << dest << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::neg(Register& dest) {
		out << "\tneg " << dest << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::imul(Memory& src, Register& dest) {
		out << "\timul " << src << ", " << dest << '\n';

		flags.reset();
		flags.set(CF);
		flags.set(OF);
	}

	void AsmEmitter::cdq() {
		out << "\tcdq\n";

		flags.reset();
	}

	void AsmEmitter::idiv(Memory& src, Register& dest) {
		out << "\tidiv " << src << ", " << dest << '\n';

		flags.reset();
	}

	void AsmEmitter::_xor(Register& src, Register& dest) {
		out << "\txor " << src << ", " << dest << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::_or(Memory& src, Register& dest) {
		out << "\tor " << src << ", " << dest << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::not(Register& src) {
		out << "\tnot " << src << '\n';
	}

	void AsmEmitter::test(Register& left, Register& right) {
		out << "\ttest " << left << ", " << right << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::cmp(Register& left, Memory& right) {
		out << "\tcmp " << left << ", " << right << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::cmp(Memory& left, Register& right) {
		out << "\tcmp " << left << ", " << right << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::setz(Register& dest) {
		out << "\tsetz " << dest << '\n';
	}

	void AsmEmitter::setnz(Register& dest) {
		out << "\tsetnz " << dest << '\n';
	}

	void AsmEmitter::setl(Register& dest) {
		out << "\tsetl " << dest << '\n';
	}

	void AsmEmitter::setl(Memory& dest) {
		out << "\tsetl " << dest << '\n';
	}

	void AsmEmitter::setg(Register& dest) {
		out << "\tsetg " << dest << '\n';
	}

	void AsmEmitter::setle(Register& dest) {
		out << "\tsetle " << dest << '\n';
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
