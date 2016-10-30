#pragma once

#include <variant>

namespace spero::util {
	/*
	 * Find the first location of a specific node on the stack
	 */
	template<class G, class Iter>
	Iter findFirst(Iter&& front, Iter&& back) {
		return std::find_if(front, back, [](auto&& elem) {
				return std::visit(compose(
						[](G&) { return true; },
						[](auto&&) { return false; }
					), elem);
			});
	}

	template<class G, class Iter>
	Iter findFirst(Iter&& front, Iter&& back) {
		return std::find_if(front, back, [](auto&& elem) {
				return std::visit(compose(
						[](G& g) {
							return std::visit(compose(
								[](T&) { return true; },
								[](auto&&) { return false; }
							), g);
						},
						[](auto&&) { return false; }
					), elem);
			});
	}

	/*
	 * Return whether the top node is of a given type
	 */
	template<class G, class Stack>
	bool at_top(Stack&& s) {
		return std::holds_alternative<G>(s.back());
	}

	template<class G, class T, class Stack>
	bool at_top(Stack&& s) {
		auto group = std::holds_alternative<G>(s.back());
		return group && std::holds_alternative<T>(std::get<G>(s.back()));
	}
}