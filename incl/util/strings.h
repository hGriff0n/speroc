#pragma once

#include <deque>

#include "spero_string.h"

namespace spero::util {

	std::deque<string::backing_type> split(String str, char ch);
	String escape(String);
	
}