#pragma once

#include <sstream>
#include <algorithm>
#include <iterator>
#include <deque>
#include <cctype>

namespace spero::util {
	std::deque<std::string> split(std::string str, char ch) {
		std::istringstream iss(str);
		std::deque<std::string> ret;
		std::string in;

		while (std::getline(iss, in, ch)) {
			// Determine the chunk of the string that isn't space
			auto front = std::find_if(in.begin(), in.end(), [](int ch) { return !std::isspace(ch); });
			auto back = std::find_if(front, in.end(), [](int ch) { return std::isspace(ch); });

			ret.emplace_back(front, back);
		}

		return std::move(ret);
	}
}