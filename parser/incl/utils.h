#pragma once

#include <string>
#include <memory>

namespace spero::util {
	std::string escape(std::string);

	/*
	 * Test if the given pointer has type `Test`
	 */
	template<class Test, class T, class=std::enable_if_t<std::is_base_of_v<T, Test>>>
	bool is_type(const std::unique_ptr<T>& ptr) {
		return dynamic_cast<Test*>(ptr.get());
	}

	/*
	 * Dynamically cast a `unique_ptr<From>` to a `unique_ptr<To>`
	 *   Deletes the original pointer iff the cast fails
	 * Returns nullptr if the cast fails
	 */
	// enable if somehow doesn't work
	template<class To, class From, class=std::enable_if_t<std::is_base_of_v<From, To>>>
	std::unique_ptr<To> dyn_cast(std::unique_ptr<From>&& f) {
		//if /*constexpr*/ (std::is_same_v<From, To>)
			//return std::move(f);

		//else {
			To* tmp = dynamic_cast<To*>(f.get());
			if (tmp) f.release();
			return std::move(std::unique_ptr<To>{ tmp });
		//}
	}

	/*
	 * Pop the top item off of the stack iff it is a `Node`
	 * Otherwise return nullptr
	 */
	template<class Node, class T, template<class, class> class Stack, class... Ts>
	std::unique_ptr<std::enable_if_t<std::is_base_of_v<T, Node>, Node>> pop(Stack<std::unique_ptr<T>, Ts...>& stack) {
		if (is_type<Node>(stack.back())) {
			auto ret = dyn_cast<Node>(std::move(stack.back()));
			stack.pop_back();
			return std::move(ret);
		}

		return nullptr;
	}

	// Find node
}