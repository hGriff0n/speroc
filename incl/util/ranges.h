#pragma once

#include <utility>

namespace spero::util {

	template<class T>
	auto range(T&& collection) {
		auto front = std::begin(collection);
		auto end = std::end(collection);

		return std::pair{ std::move(front), std::move(end) };
	}

}