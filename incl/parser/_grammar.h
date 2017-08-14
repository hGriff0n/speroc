#pragma once
#pragma warning(disable : 4503)

#include "pegtl.hpp"

namespace spero::parser::grammar {
	using namespace tao::pegtl;
#define pstr(x) TAOCPP_PEGTL_STRING((x))
#define key(x) seq<pstr((x)), not_at<ascii::identifier_other>, ig_s> {}


	struct program : success {};
}