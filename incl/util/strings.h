#pragma once

#include <sstream>
#include <algorithm>
#include <iterator>
#include <deque>
#include <cctype>

namespace spero::util {

	std::deque<std::string> split(std::string str, char ch);
	//std::string escape(std::string);
	
}