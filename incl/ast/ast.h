#pragma once

#include <optional>
#include <string>
#include <vector>

#include "ast/node_defs.h"


namespace spero::compiler::ast {
	// Literals
	struct Byte {
		int val;
		Byte(const std::string&, int base);
	};
	struct Int {
		int val;
		Int(const std::string&);
	};
	struct Float {
		float val;
		Float(const std::string&);
	};
	struct String {
		std::string val;
		String(const std::string&);
	};
	struct Char {
		char val;
		Char(char);
	};
	struct Bool {
		bool val;
		Bool(bool);
	};
	struct Tuple {
		std::vector<astnode> val;

		template<class T> Tuple(T&& front, T&& end) {
			std::move(front, end, std::back_inserter(this->val));
		}
	};

	// Bindings
	struct Var {};
	struct Op {};
	struct Type {};
}

namespace spero::util {
	template<class Stream>
	Stream& buffer(Stream& s, int depth) {
		return s << std::string(depth, ' ');
	}

	template<class Stream>
	Stream& pretty_print(spero::compiler::astnode& root, Stream& s, int depth = 0) {
		using namespace spero::compiler;
		using namespace compiler::ast;

		buffer(s, depth);

		std::visit(compose(
			[depth, &s](litnode& lits) {
				std::visit(compose(
					[&s](Byte& b) { s << "Byte found with val " << b.val << "\n"; },
					[&s](Int& i) { s << "Int found with val " << i.val << "\n"; },
					[&s](Float& f) { s << "Float found with val " << f.val << "\n"; },
					[&s](String& str) { s << "String found with val " << str.val << "\n"; },
					[&s](Char& c) { s << "Char found with val " << c.val << "\n"; },
					[&s](Bool& b) { s << "Bool found with val " << (b.val ? "true" : "false") << "\n"; },
					[depth, &s](Tuple& t) {
						s << "Tuple found of size " << t.val.size() << "\n";
						for (auto elem : t.val)
							pretty_print(elem, s, depth + 1);
					}
				), lits);
			},
			[&s](Sentinel&) {
				s << "A sentinel node still exists in the stack\n";
			},
			[&s](Keyword& k) {
				s << "Keyword: k\n";
			}), root);

		return s;
	}
}