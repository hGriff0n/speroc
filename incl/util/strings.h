#pragma once

#include <deque>
#include <string>

namespace spero::util {

	std::deque<std::string> split(std::string str, char newline_ch);
	std::string escape(std::string str);
	
}