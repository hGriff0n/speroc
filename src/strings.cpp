
#include "util/strings.h"

#include <algorithm>
#include <cctype>
#include <deque>
#include <sstream>

namespace spero::util {

	std::deque<std::string> split(std::string str, char newline_ch) {
		std::basic_stringstream<std::string::traits_type::char_type> iss(str);
		std::deque<std::string> ret;
		std::string in;

		// Determine the chunk of the string that isn't space
		while (std::getline(iss, in, newline_ch)) {
			auto front = std::find_if(in.begin(), in.end(), [](int ch) { return !std::isspace(ch); });
			auto back = std::find_if(front, in.end(), [](int ch) { return std::isspace(ch); });

			ret.emplace_back(front, back);
		}

		return std::move(ret);
	}

	std::string escape(std::string str) {
		return str;
	}

}