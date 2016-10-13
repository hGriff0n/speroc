#pragma once

#include <variant>

namespace spero::util {
	template<class T, class Iter>
	Iter findFirst(Iter&& front, Iter&& back) {
		return std::find_if(front, back, [](auto&& elem) {
			return std::visit(compose(
				[](T&) { return true; },
				[](auto&&) { return false; }
			), elem);
		});
	}

	// Unknown
	template<class T, class Stack>
	bool topIs(Stack&& s) {
		return std::visit(compose(
			[](T&) { return true; },
			[](auto&&) { return false; }
		), s.back());
	}

	template<class T, class N, class Stack>
	bool topIs(Stack&& s) {
		return std::visit(compose(
			[](T& type) {
				return std::visit(compose(
					[](N&) { return true; },
					[](auto&&) { return false; }), type);
			},
			[](auto&&) { return false; }
		), s.back());
	}
}
