#pragma once

#include "actions.h"

namespace spero::parser::control {
	template<class Rule>
	struct control : pegtl::normal<Rule> {};

	template<class Rule>
	struct tracer : pegtl::normal<Rule> {
		template<class Input, class... States>
		static void start(const Input& in, States&&...) {
			std::cerr << pegtl::position_info(in) << "  start  " << pegtl::internal::demangle<Rule>() << '\n';
		}

		template<class Input, class... States>
		static void success(const Input& in, States&&...) {
			std::cerr << pegtl::position_info(in) << "  success " << pegtl::internal::demangle<Rule>() << '\n';
		}

		template<class Input, class... States>
		static void failure(const Input& in, States&&...) {
			std::cerr << pegtl::position_info(in) << "  failure " << pegtl::internal::demangle<Rule>() << '\n';
		}
	};
}