#include "driver/AnalysisDriver.h"

#include "parser/actions.h"

// Passes
#include "analysis/VarDeclPass.h"
#include "analysis/VarRefPass.h"
#include "analysis/BasicTypingPass.h"
#include "codegen/LlvmIrGenerator.h"

using namespace spero;
using namespace spero::compiler;
using namespace spero::parser;

AnalysisDriver::AnalysisDriver(CompilationState& state, analysis::AllTypes& types)
	: context{ state.getContext() }, state{ state }, decls{ types }
{}

#define TIMER(name) auto _ = state.timer(name)
void AnalysisDriver::parseInput(parser::ParsingMode parser_mode, const std::string& input) {
	TIMER("parsing");

	bool success = false;
	switch (parser_mode) {
		case parser::ParsingMode::FILE:
			success = tao::pegtl::parse<grammar::program, actions::action>(tao::pegtl::file_input<>{ input }, ast, state);
			break;
		case parser::ParsingMode::STRING:
			success = tao::pegtl::parse<grammar::program, actions::action>(tao::pegtl::string_input<>{ input, "speroc" }, ast, state);
			break;
		default:
			state.log(ID::err, "Invalid parsing mode passed to AnalysisDriver::parseInput");
			success = true;
			break;
	}

	if (!success) {
		state.log(ID::err, "Error in parsing of input");
	}
}

#define RUN_PASS(PassType, ...) { PassType pass { __VA_ARGS__ }; ast::visit(pass, ast); }

void AnalysisDriver::analyzeAst() {
	if (!state.failed()) {
		TIMER("ast_analysis");

		RUN_PASS(analysis::VarDeclPass, state, decls);
		RUN_PASS(analysis::VarRefPass, state, decls);
	}
}

void AnalysisDriver::translateAstToLlvm() {
	if (!state.failed()) {
		TIMER("llvm_ir_translation");

		gen::LlvmIrGenerator visitor{ decls.arena, state };
		ast::visit(visitor, ast);
		translation_unit = std::move(visitor.finalize());
	}
}

void AnalysisDriver::optimizeLlvm() {
	// TODO: Implement llvm ir optimization
	if (!state.failed() && state.optimizationLevel() == OptimizationLevel::ALL) {
		TIMER("llvm_ir_optimization");
	}
}

#undef RUN_PASS
#undef TIMER