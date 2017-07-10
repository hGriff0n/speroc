#pragma once

#include "actions.h"

namespace spero::parser::control {
	template<class Rule>
	struct control : tao::pegtl::normal<Rule> {};

	template<class Rule>
	struct tracer : tao::pegtl::normal<Rule> {
		template<class Input, class... States>
		static void start(const Input& in, States&&...) {
			std::cerr << tao::pegtl::position_info(in) << "  start  " << tao::pegtl::internal::demangle<Rule>() << '\n';
		}

		template<class Input, class... States>
		static void success(const Input& in, States&&...) {
			std::cerr << tao::pegtl::position_info(in) << "  success " << tao::pegtl::internal::demangle<Rule>() << '\n';
		}

		template<class Input, class... States>
		static void failure(const Input& in, States&&...) {
			std::cerr << tao::pegtl::position_info(in) << "  failure " << tao::pegtl::internal::demangle<Rule>() << '\n';
		}
	};
}