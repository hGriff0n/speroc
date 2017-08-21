#pragma once

#include <string>
#include <memory>
#include <algorithm>

namespace spero::util {
	std::string escape(std::string);

	template<class Base, class Derived, class T = void>
	using enable_if_base = std::enable_if_t<std::is_base_of_v<Base, Derived>, T>;

	/*
	 * Test if the given pointer has type `Test`
	 */
	template<class Test, class T, class=enable_if_base<T, Test>>
	bool is_type(const std::unique_ptr<T>& ptr) {
		return dynamic_cast<Test*>(ptr.get());
	}

	// Specialization for working with the stack
	template<class Node, class Stack>
	bool at_node(Stack& stack) {
		return std::size(stack) != 0 && is_type<Node>(stack.back());
	}

	/*
	 * View a unique_ptr as another to perform some operations on the underlying
	 */
	template<class Node, class T, class=enable_if_base<T, Node>>
	Node* view_as(std::unique_ptr<T>& ptr) {
		return dynamic_cast<Node*>(ptr.get());
	}

	/*
	 * Dynamically cast a `unique_ptr<From>` to a `unique_ptr<To>`
	 *   Destroys the passed pointer if the cast succeeds
	 * Returns nullptr if the cast fails
	 */
	template<class To, class From, class=enable_if_base<From, To>>
	std::unique_ptr<To> dyn_cast(std::unique_ptr<From> f) {
		if constexpr (std::is_same_v<From, To>)
			return std::move(f);

		else {
			To* tmp = dynamic_cast<To*>(f.get());
			if (tmp) f.release();
			return std::unique_ptr<To>{ tmp };
		}
	}

	/*
	 * Pop the top item off of the stack iff it is a `Node`
	 * Otherwise return nullptr
	 */
	template<class Node, class T, template<class, class...> class Stack, class... Ts>
	std::unique_ptr<enable_if_base<T, Node, Node>> pop(Stack<std::unique_ptr<T>, Ts...>& stack) {
		if (std::size(stack) != 0 && is_type<Node>(stack.back())) {
			auto ret = dyn_cast<Node>(std::move(stack.back()));
			stack.pop_back();
			return std::move(ret);
		}

		return nullptr;
	}

	/*
	 * Finds the first occurance of the given node
	 */
	template<class Node, class Iter>
	auto findFirst(Iter front, Iter end) {
		return std::find_if(front, end, [](auto&& t) { return is_type<Node>(t); });
	}
	template<class Node, class Stack>
	auto findFirst(Stack& stack) {
		return findFirst<Node>(std::begin(stack), std::end(stack));
	}

	/*
	 * Constructs a deque and fills it with the requested node type
	 */
	template<class T, class Stack>
	auto popSeq(Stack& s) {
		std::deque<std::unique_ptr<T>> ret;
		while (util::at_node<T>(s))
			ret.push_front(util::pop<T>(s));
		return std::move(ret);
	}
}