#pragma once

#include <type_traits>
#include <variant>

// Taken from lambda_utils gist
namespace spero::util {
	template <typename... lambda_ts>
	struct composer_t;

	template <typename lambda_t>
	struct composer_t<lambda_t> : lambda_t {
		composer_t(const lambda_t& lambda) : lambda_t{ lambda } {}
		composer_t(lambda_t&& lambda) : lambda_t{ std::move(lambda) } {}

		using lambda_t::operator();
	};

	template <typename lambda_t, typename... more_lambda_ts>
	struct composer_t<lambda_t, more_lambda_ts...> : lambda_t, composer_t<more_lambda_ts...> {
		using super_t = composer_t<more_lambda_ts...>;

		template <typename... lambda_ts>
		composer_t(const lambda_t& lambda, lambda_ts&&... more_lambdas) : lambda_t{ lambda }, super_t{ std::forward<lambda_ts>(more_lambdas)... } {}
		template <typename... lambda_ts>
		composer_t(lambda_t&& lambda, lambda_ts&&... more_lambdas) : lambda_t{ std::move(lambda) }, super_t{ std::forward<lambda_ts>(more_lambdas)... } {}

		using lambda_t::operator();
		using super_t::operator();
	};	
}

template <typename... lambda_ts>
auto compose(lambda_ts&&... lambdas) {
	return spero::util::composer_t<std::decay_t<lambda_ts>...>{std::forward<lambda_ts>(lambdas)...};
}
