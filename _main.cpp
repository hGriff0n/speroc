
#include <iostream>
#include <unordered_map>
#include <cctype>

#include "incl/parser/_grammar.h"

template<class Stream> Stream& getMultiline(Stream& in, std::string& s);

int main(int argc, const char* argv) {
	std::string input;

	while (std::cout << "> " && getMultiline(std::cin, input)) {
		if (input.substr(0, 2) == ":q") break;

		auto succ = tao::pegtl::parse<spero::parser::grammar::program>(tao::pegtl::string_input<>{ input, "speroc:repl" });

		std::cout << "Success: " << succ << '\n';
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