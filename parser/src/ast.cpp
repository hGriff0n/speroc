#include "ast.h"
//#include "util/utils.h"

#include <iterator>

//using namespace spero::util;

namespace spero::compiler::ast {
	#define PRETTY_PRINT(typ) std::string typ::pretty_print(size_t buf)

	//
	// Parent Nodes
	//
	Ast::Ast() {}
	Token::Token(KeywordType t) : val{ t } {}
	Token::Token(PtrStyling t) : val{ t } {}
	Token::Token(VarianceType t) : val{ t } {}
	Token::Token(RelationType t) : val{ t } {}
	Token::Token(VisibilityType t) : val{ t } {}
	Token::Token(BindingType t) : val{ t } {}
	Token::Token(UnaryType t) : val{ t } {}

	PRETTY_PRINT(Token) {
		if (std::holds_alternative<KeywordType>(val))
			return std::get<KeywordType>(val)._to_string();
		else if (std::holds_alternative<PtrStyling>(val))
			return std::get<PtrStyling>(val)._to_string();
		else if (std::holds_alternative<VarianceType>(val))
			return std::get<VarianceType>(val)._to_string();
		else if (std::holds_alternative<RelationType>(val))
			return std::get<RelationType>(val)._to_string();
		else if (std::holds_alternative<VisibilityType>(val))
			return std::get<VisibilityType>(val)._to_string();
		else if (std::holds_alternative<BindingType>(val))
			return std::get<BindingType>(val)._to_string();
		else if (std::holds_alternative<UnaryType>(val))
			return std::get<UnaryType>(val)._to_string();
		else
			return "Empty";
	}

	Stmt::Stmt() {}
	ValExpr::ValExpr() {}
	PRETTY_PRINT(Ast) {
		return "";
	}


	//
	// Literals
	//
	
	Byte::Byte(const std::string& num, int base) : val{ std::stoul(num, nullptr, base) } {}
	PRETTY_PRINT(Byte) {
		return std::string(buf, ' ') + "Byte: " + std::to_string(val);
	}

	
	Int::Int(const std::string& num) : val{ std::stoi(num) } {}
	PRETTY_PRINT(Int) {
		return std::string(buf, ' ') + "Int: " + std::to_string(val);
	}

	
	Float::Float(const std::string& num) : val{ std::stof(num) } {}
	PRETTY_PRINT(Float) {
		return std::string(buf, ' ') + "Float: " + std::to_string(val);
	}

	
	String::String(const std::string& str) : val{ str } {}
	PRETTY_PRINT(String) {
		return std::string(buf, ' ') + "String: " + val;
	}


	Char::Char(char c) : val{ c } {}
	PRETTY_PRINT(Char) {
		return std::string(buf, ' ') + "Char: " + val;
	}


	Bool::Bool(bool b) : val{ b } {}
	PRETTY_PRINT(Bool) {
		return std::string(buf, ' ') + "Bool: " + (val ? "true" : "false");
	}


	//
	// Bindings
	//
	BasicBinding::BasicBinding(std::string n, BindingType t) : name{ n }, type{ t } {}
	PRETTY_PRINT(BasicBinding) {
		return std::string(buf, ' ') + "Binding: " + name + "(" + type._to_string() + ")";
	}

	QualBinding::QualBinding(ptr<BasicBinding> b) {
		add(std::move(b));
	}
	void QualBinding::add(ptr<BasicBinding> b) {
		val.emplace_back(std::move(b));
	}
	PRETTY_PRINT(QualBinding) {
		std::string ret(buf, ' ');
		ret += "QualBinding";
		
		for (auto&& b : val)
			ret += "\n" + b->pretty_print(buf + 1);

		return ret;
	}


	//
	// Atoms
	//
	FnCall::FnCall(node expr) : FnCall(std::move(expr), nullptr, nullptr, nullptr) {}
	FnCall::FnCall(node expr, ptr<TypeExt> anon, ptr<Tuple> args, ptr<Array> inst)
		: caller{ std::move(expr) }, anon{ std::move(anon) }, args{ std::move(args) }, inst{ std::move(inst) } {}
	PRETTY_PRINT(FnCall) {
		return "FnCall";
	}

	#undef PRETTY_PRINT
}
