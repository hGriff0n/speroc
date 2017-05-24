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

	void AsmEmitter::emit(std::string&& str) {
		out << str << '\n';
	}

	void AsmEmitter::emitLabel(std::string&& label) {
		out << label << ":\n";
	}

	void AsmEmitter::push(const Register& r) {
		out << "\tpush " << r << '\n';
		
		flags.reset();
	}

	void AsmEmitter::pop(const Register& r) {
		out << "\tpop " << r << '\n';
		
		flags.reset();
	}

	void AsmEmitter::popByte(size_t n) {
		Register esp{ "esp" };
		out << "\tadd $" << (n * 4) << ", " << esp << '\n';

		flags.reset();
	}

	void AsmEmitter::mov(const Register& src, const Register& dest) {
		out << "\tmov " << src << ", " << dest << '\n';

		flags.reset();
	}

	void AsmEmitter::mov(int num, const Register& dest) {
		out << "\tmov $" << num << ", " << dest << '\n';
		
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

	void AsmEmitter::sub(const Register& src, const Memory& dest) {
		out << "\tsub " << src << ", " << dest << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::neg(const Register& dest) {
		out << "\tneg " << dest << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::imul(const Memory& src, const Register& dest) {
		out << "\timul " << src << ", " << dest << '\n';

		flags.reset();
		flags.set(CF);
		flags.set(OF);
	}

	void AsmEmitter::cdq() {
		out << "\tcdq\n";

		flags.reset();
	}

	void AsmEmitter::idiv(const Memory& src, const Register& dest) {
		out << "\tidiv " << src << ", " << dest << '\n';

		flags.reset();
	}

	void AsmEmitter::_xor(const Register& src, const Register& dest) {
		out << "\txor " << src << ", " << dest << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::_or(const Memory& src, const Register& dest) {
		out << "\tor " << src << ", " << dest << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::not(const Register& src) {
		out << "\tnot " << src << '\n';
	}

	void AsmEmitter::test(const Register& left, const Register& right) {
		out << "\ttest " << left << ", " << right << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::cmp(const Register& left, const Memory& right) {
		out << "\tcmp " << left << ", " << right << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::cmp(const Memory& left, const Register& right) {
		out << "\tcmp " << left << ", " << right << '\n';

		flags |= ALL_FLAGS;
	}

	void AsmEmitter::setz(const Register& dest) {
		out << "\tsetz " << dest << '\n';
	}

	void AsmEmitter::setnz(const Register& dest) {
		out << "\tsetnz " << dest << '\n';
	}

	void AsmEmitter::setl(const Register& dest) {
		out << "\tsetl " << dest << '\n';
	}

	void AsmEmitter::setl(const Memory& dest) {
		out << "\tsetl " << dest << '\n';
	}

	void AsmEmitter::setg(const Register& dest) {
		out << "\tsetg " << dest << '\n';
	}

	void AsmEmitter::setle(const Register& dest) {
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