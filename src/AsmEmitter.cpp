#include "codegen/AsmEmitter.h"

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

	void AsmEmitter::push(_Register& r) {
		out << "\tpush " << r << '\n';

		flags.reset();
	}

	void AsmEmitter::pushByte(size_t n) {
		_Register esp{ "esp" };
		out << "\tadd $" << (n * 4) << ", " << esp << '\n';

		flags.reset();
	}

	void AsmEmitter::pop(_Register& r) {
		out << "\tpop " << r << '\n';

		flags.reset();
	}

	void AsmEmitter::popByte(size_t n) {
		_Register esp{ "esp" };
		out << "\tadd $" << (n * 4) << ", " << esp << '\n';

		flags.reset();
	}

	void AsmEmitter::mov(_Register& src, _Register& dest) {
		out << "\tmov " << src << ", " << dest << '\n';

		flags.reset();
	}

	void AsmEmitter::mov(Literal num, _Register& dest) {
		out << "\tmov " << num << ", " << dest << '\n';

		flags.reset();
	}

	void AsmEmitter::mov(Memory& src, _Register& dest) {
		out << "\tmov " << src << ", " << dest << '\n';

		flags.reset();
	}

	void AsmEmitter::mov(_Register& src, Memory& dest) {
		out << "\tmov " << src << ", " << dest << '\n';

		flags.reset();
	}

	void AsmEmitter::movzx(_Register& src, _Register& dest) {
		out << "\tmovzx " << src << ", " << dest << '\n';

		flags.reset();
	}

	void AsmEmitter::add(Memory& src, _Register& dest) {
		out << "\tadd " << src << ", " << dest << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::add(Literal num, _Register& dest) {
		out << "\tadd " << num << ", " << dest << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::sub(_Register& src, Memory& dest) {
		out << "\tsub " << src << ", " << dest << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::neg(_Register& dest) {
		out << "\tneg " << dest << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::imul(Memory& src, _Register& dest) {
		out << "\timul " << src << ", " << dest << '\n';

		flags.reset();
		flags.set(CF);
		flags.set(OF);
	}

	void AsmEmitter::cdq() {
		out << "\tcdq\n";

		flags.reset();
	}

	void AsmEmitter::idiv(Memory& src) {
		out << "\tidiv " << src << '\n';

		flags.reset();
	}

	void AsmEmitter::_xor(_Register& src, _Register& dest) {
		out << "\txor " << src << ", " << dest << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::_or(Memory& src, _Register& dest) {
		out << "\tor " << src << ", " << dest << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::not(_Register& src) {
		out << "\tnot " << src << '\n';
	}

	void AsmEmitter::test(_Register& left, _Register& right) {
		out << "\ttest " << left << ", " << right << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::cmp(_Register& left, Memory& right) {
		out << "\tcmp " << left << ", " << right << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::cmp(Memory& left, _Register& right) {
		out << "\tcmp " << left << ", " << right << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::setz(_Register& dest) {
		out << "\tsetz " << dest << '\n';
	}

	void AsmEmitter::setnz(_Register& dest) {
		out << "\tsetnz " << dest << '\n';
	}

	void AsmEmitter::setl(_Register& dest) {
		out << "\tsetl " << dest << '\n';
	}

	void AsmEmitter::setl(Memory& dest) {
		out << "\tsetl " << dest << '\n';
	}

	void AsmEmitter::setle(_Register& dest) {
		out << "\tsetle " << dest << '\n';
	}

	void AsmEmitter::setg(_Register& dest) {
		out << "\tsetg " << dest << '\n';
	}

	void AsmEmitter::setge(_Register& dest) {
		out << "\tsetge " << dest << '\n';
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
