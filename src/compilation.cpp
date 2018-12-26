#include "compilation.h"

#pragma warning(push, 0)
#pragma warning(disable:4996)
#include <llvm/Support/FileSystem.h>
#pragma warning(pop)

#include "parser/actions.h"
#include "analysis/VarDeclPass.h"
#include "analysis/VarRefPass.h"
#include "analysis/BasicTypingPass.h"

#include "codegen/LlvmIrGenerator.h"

namespace spero::compiler {
	using namespace parser;

	template<class Fn>
	Stack parse_impl(CompilationState& state, Fn&& parse) {
		// Setup parser state
		Stack ast;

		// Perform a parsing run, switching on whether the input is a file or not
		auto succ = parse(ast, state);
		if (!succ) {
			state.log(compiler::ID::err, "Error in parsing");
		}

		// Return the completed ast
		return std::move(ast);
	}

	Stack parse(std::string input, CompilationState& state) {
		using namespace spero::parser;

		return parse_impl(state, [&input](Stack& ast, CompilationState& state) {
			return tao::pegtl::parse<grammar::program, actions::action>(tao::pegtl::string_input<>{ input, "speroc:repl" }, ast, state);
		});
	}

	Stack parseFile(std::string file, CompilationState& state) {
		using namespace spero::parser;

		return parse_impl(state, [&file](Stack& ast, CompilationState& state) {
			return tao::pegtl::parse<grammar::program, actions::action>(tao::pegtl::file_input<>{ file }, ast, state);
		});
	}

#define RUN_PASS_ARGS(PassType, ...) { PassType pass{ __VA_ARGS__ }; ast::visit(pass, ast_stack); }
#define RUN_PASS(PassType) RUN_PASS_ARGS(PassType, state, dictionary)

	// Perform the various analysis stages
	MIR_t analyze(parser::Stack& ast_stack, CompilationState& state, analysis::AllTypes& type_list) {
		analysis::AnalysisState dictionary{ type_list };

		RUN_PASS(analysis::VarDeclPass);
		// RUN_PASS(analysis::BasicTypingPass);
		RUN_PASS(analysis::VarRefPass);

		return std::move(dictionary.arena);
	}

#undef RUN_PASS

	llvm::Module* backend(MIR_t, parser::Stack& ast_stack, CompilationState& state, bool using_repl) {
		gen::LlvmIrGenerator visitor{ state };

		// Temporary hack since we have to put stuff in a function to see it in the repl
		if (using_repl) {
			std::vector<llvm::Type*> args;
			auto ft = llvm::FunctionType::get(llvm::Type::getInt32Ty(state.getContext()), args, false);
			auto fn = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "jitfunc", visitor.finalize());
			visitor.getBuilder().SetInsertPoint(llvm::BasicBlock::Create(state.getContext(), "entry", fn));
		}

		ast::visit(visitor, ast_stack);

		if (using_repl) {
			visitor.createDefaultRetInst();
		}

		return visitor.finalize();
	}

	// Perform the final compilation stages (produces llvm ir file)
	void codegen(llvm::Module* ir, const std::string& input_file, const std::string& output_file, spero::compiler::CompilationState& state) {
		std::error_code ec;
		llvm::raw_fd_ostream output{ output_file, ec, llvm::sys::fs::F_None };

		// TODO: The target layout/triple strings should be configurable (ie. get them from CompilationState)
		ir->setDataLayout(state.targetDataLayout());
		ir->setTargetTriple(state.targetTriple());
		ir->print(output, nullptr);
	}
}


// Full compilation implementation
bool spero::compile(spero::compiler::CompilationState& state, spero::parser::Stack& ast_stack) {
	return spero::compile(state, ast_stack, [](auto&&) {});
}
