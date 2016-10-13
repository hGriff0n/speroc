#pragma once

#include "ast/node_defs.h"

#include <optional>
#include <string>
#include <vector>
#include <memory>		// using std::shared_ptr as a cycle "breaker"


namespace spero::compiler::ast {
	// Bindings
	struct BasicName {
		std::string name;
		bool is_typ;
		BasicName(const std::string&);
	};
	struct Operator {
		std::string op;
		Operator(const std::string&);
	};
	struct NamePath {
		std::vector<BasicName> inter;
	};
	struct QualName {
		NamePath path;
		BasicName name;
	};
	struct Pattern {

	};

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
			std::move(front, end, std::back_inserter(val));
		}
	};
	struct Array {
		std::vector<astnode> val;

		template<class T> Array(T&& front, T&& end) {
			std::move(front, end, std::back_inserter(val));
		}
	};
	struct Type {
		QualName type;
		PtrStyling ptr;
		std::optional<Array> gens;
		bool mut;
	}; 
	struct FnObj {
		bool forward;
		std::shared_ptr<astnode> body;
		std::optional<Tuple> arguments;
		std::optional<Type> return_type;
	};

	// Atoms
	struct FnCall {
		QualName fm;
		std::optional<Array> gens;
		std::optional<Tuple> args;
	};
	struct Scope {
		std::vector<astnode> val;

		template<class T> Scope(T&& front, T&& end) {
			std::move(front, end, std::back_inserter(val));
		}
	};
	// Error here
	struct Case {
		std::vector<Pattern> vars;
		std::shared_ptr<astnode> expr;
	};
	// Error here
	struct Match {
		std::shared_ptr<astnode> val;
		std::vector<Case> cases;
	};

	// Decorators
	struct Annotation {
		BasicName annotation;
		std::optional<Tuple> args;
	};
	struct ModDecl {
		std::vector<BasicName> modules;
	};
	struct TypeTuple {
		std::vector<Type> types;
	};
	struct TypeInfer {
		std::optional<TypeTuple> fn_args;
		Type val_type;
	};
	struct GenSubtype {
		SubtypeRelation relation;
		Type type;
	};
	struct GenType {
		BasicName type_name;
		Variance variant;
		std::optional<GenSubtype> subtype;
	};
	// Error here
	struct GenValue {
		BasicName var_name;
		std::optional<Type> subtype;
		std::shared_ptr<astnode> value;
	};
	struct GenArray {
		std::vector<std::variant<GenType, GenValue>> gen_entries;
	};

	// Assignment
	struct ADT {
		BasicName name;
		std::optional<TypeTuple> accepts;
	};
	struct VarAssign {
		bool mut;
		// pattern or op
		std::optional<TypeInfer> inference;
		std::shared_ptr<astnode> expr;
	};
	struct TypeAssign {
		BasicName type_name;
		std::optional<GenArray> generics;
		std::vector<std::variant<ADT, Tuple>> constructors;
		Scope type_body;
	};
	struct Assignment {
		Keyword visiblity;
		std::variant<VarAssign, TypeAssign> assignment;
	};

	// Expressions
	struct Index {
		std::shared_ptr<astnode> lhs, rhs;
		std::optional<TypeInfer> infer;
	};
	struct InfixOp {
		std::shared_ptr<astnode> lhs, rhs;
		Operator op;
	};
	struct Branch {
		std::shared_ptr<astnode> test, branch;
	};
	// Error here
	struct IfStmt {
		Branch if_true;
		std::vector<Branch> elsif_Branches;
		std::shared_ptr<astnode> else_branch;
	};
	struct UsePathElem {
		std::vector<BasicName> modules;
	};
	struct UseImport {
		BasicName import;
		std::optional<BasicName> rebind;
	};
	struct UseStmt {
		std::vector<UsePathElem> use_path;
		std::vector<UseImport> imports;
		bool import_all;
	};
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
			[&s](auto&&) { s << "The AST is currently being worked on and will be back shortly\n"; }
		), root);

		return s;
	}
}
