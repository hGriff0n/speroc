
#include "util/strings.h"

#include <cctype>
#include <deque>
#include <sstream>

namespace spero::util {

	std::deque<string::backing_type> split(String str, char ch) {
		std::basic_stringstream<string::char_type> iss(str);
		std::deque<string::backing_type> ret;
		string::backing_type in;

		while (std::getline(iss, in, ch)) {
			// Determine the chunk of the string that isn't space
			auto front = std::find_if(in.begin(), in.end(), [](int ch) { return !std::isspace(ch); });
			auto back = std::find_if(front, in.end(), [](int ch) { return std::isspace(ch); });

			ret.emplace_back(front, back);
		}

		return std::move(ret);
	}

	String escape(String str) {
		return str;
	}

}