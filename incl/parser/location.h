#pragma once

#include "pegtl/internal/iterator.hpp"

namespace spero::compiler {

	struct Location {
		size_t line_num;
		size_t byte;
		const char* data;
		size_t byte_in_data;
		std::string source;

		template<class T>
		inline Location(const tao::pegtl::internal::iterator& in_iter, T&& in_source)
		    : byte{ in_iter.byte_in_line },
		      line_num{ in_iter.line },
		      data{ in_iter.data },
		      byte_in_data{ in_iter.byte_in_line },
		      source{ std::forward<T>(in_source) }
		{}
	};

	template<class Stream>
	inline Stream& operator<<(Stream& s, const Location& loc) {
		return s << loc.source << ':' << loc.line_num << ':' << loc.byte;
	}

	/*inline std::string to_string(const Location& loc) {
		std::ostringstream o;
		o << loc;
		return o.str();
	}*/

}