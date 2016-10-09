

#define CATCH_CPP14_OR_GREATER
#define CATCH_CONFIG_RUNNER

#include "catch.hpp"

#include "pegtl/analyze.hh"
#include "parser/control.h"
#include "cmd_line.h"
#include "util/utils.h"

#include <iostream>

const size_t issues = pegtl::analyze<spero::parser::grammar::program>();

int main(int argc, const char* argv[]) {
	using namespace spero::parser;
	//auto opts = spero::cmd::getOptions();

	std::cout << "Finished ";

	auto input = std::string{ "0xf 34.35 '\\n'" };

	Stack s{};

	std::cout << pegtl::parse_string<grammar::program, actions::action>(input, "me", s) << "\n";

	for (auto elem : s) {
		using namespace spero::compiler;
		std::visit(compose(
			[](litnode& lits) {
				std::visit(compose(
					[](ast::Byte& b) { std::cout << "Byte found with val " << b.val << "\n"; },
					[](ast::Int& i) { std::cout << "Int found with val " << i.val << "\n"; },
					[](ast::Float& f) { std::cout << "Float found with val " << f.val << "\n"; },
					[](ast::String& s) { std::cout << "String found with val " << s.val << "\n"; },
					[](ast::Char& c) { std::cout << "Char found with val " << c.val << "\n"; }
				), lits);
			},
			[](auto&& any) {
				std::cout << "Unknown element\n"
			}), elem);

	}
	std::cin.get();
}
