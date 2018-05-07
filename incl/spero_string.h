#pragma once

#include <string>

#include <boost/flyweight.hpp>

namespace spero {

	namespace string {
		using backing_type = std::string;
		using char_type = backing_type::traits_type::char_type;
	}

	using String = boost::flyweight<std::string>;

}


// Override `std::hash` for usage in `std::unordered_map`
namespace std {

	template<>
	struct hash<spero::String> {
		using result_type = std::size_t;
		using argument_type = spero::String;

		result_type operator()(const spero::String& x) const {
			std::hash<const argument_type::value_type*> h;
			return h(&x.get());
		}
	};

}