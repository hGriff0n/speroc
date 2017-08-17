
#include <iostream>
#include <unordered_map>
#include <cctype>

#include "incl/parser/_actions.h"
#include <pegtl/analyze.hpp>


template<class Stream> Stream& getMultiline(Stream& in, std::string& s);
using namespace tao;

int main(int argc, const char* argv) {
	std::string input;

	std::cout << "nissues: " << pegtl::analyze<spero::parser::grammar::program>() << '\n';

	while (std::cout << "\n> " && getMultiline(std::cin, input)) {
		if (input.substr(0, 2) == ":q") break;

		spero::parser::Stack res;

		try {
			using namespace spero::parser;
			auto succ = pegtl::parse<grammar::program, actions::action>(
				pegtl::string_input<>{ input, "speroc:repl" }, res);

			std::cout << "Succeeded? " << (succ ? "yes" : "no") << "\n\n";

			for (auto&& ast : res) {
				if (ast) {
					ast->prettyPrint(std::cout, 0) << '\n';
				}
			}

		} catch (std::exception& e) {
			std::cout << e.what() << '\n';
		}
	}
}

template <class Stream>
Stream& getMultiline(Stream& in, std::string& s) {
	std::getline(in, s);
	if (s == ":q") {
		return in;
	}

	std::string tmp;
	while (std::getline(in, tmp)) {
		if (tmp == "") {
			return in;
		}

		s += "\n" + tmp;
	}

	return in;
}