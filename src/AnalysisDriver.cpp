#include "driver/AnalysisDriver.h"

// NOTE: Always surround llvm includes with these commands to disable warning reporting
// As they are both very numerous, not our concern, and actually prevent compilation (due to -werror)
#pragma warning(push, 0)
#pragma warning(disable:4996)
//#pragma warning(pop)
#include <llvm/Passes/PassBuilder.h>
#pragma warning(pop)

#include "parser/actions.h"

// Passes
#include "analysis/VarDeclPass.h"
#include "analysis/VarRefPass.h"
#include "analysis/BasicTypingPass.h"
#include "codegen/LlvmIrGenerator.h"

using namespace spero;
using namespace spero::compiler;
using namespace spero::parser;

struct spero::compiler::Optimizer {
	llvm::PassBuilder builder;

	// Construct all of the analysis managers
	// TODO: Figure out what each does
	llvm::LoopAnalysisManager loop_analysis;
	llvm::FunctionAnalysisManager function_analysis;
	llvm::CGSCCAnalysisManager cGSCC_analysis;
	llvm::ModuleAnalysisManager module_analysis;

	// The optimization pass manager
	llvm::ModulePassManager driver;
};

AnalysisDriver::AnalysisDriver(CompilationState& state)
	: context{ state.getContext() }, state{ state }, opt{ new Optimizer() }
{
	// Register the core spero types/etc.
	// TODO: Replace with more generalized registration code
	decls.loadModuleTypes(analysis::getCoreTypeList());

	// Link and register all of the optimization passes
	// TODO: Clean this up a bit
	auto&[builder, loop_analysis, function_analysis, cGSCC_analysis, module_analysis, driver] = *opt;
	builder.registerModuleAnalyses(module_analysis);
	builder.registerCGSCCAnalyses(cGSCC_analysis);
	builder.registerFunctionAnalyses(function_analysis);
	builder.registerLoopAnalyses(loop_analysis);
	builder.crossRegisterProxies(loop_analysis, function_analysis, cGSCC_analysis, module_analysis);

	// Create the driver with a default optimization level
	// TODO: Update this framework when we actually implement the '-O' option
	// We'd really just need to reassign the driver, but the whole system would do some rework then
	driver = builder.buildPerModuleDefaultPipeline(llvm::PassBuilder::OptimizationLevel::O1);
}

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

		gen::LlvmIrGenerator visitor{ std::move(translation_unit), decls.arena, state };
		ast::visit(visitor, ast);
		translation_unit = std::move(visitor.finalize());
	}
}

void AnalysisDriver::optimizeLlvm() {
	if (!state.failed() && state.optimizationLevel() == OptimizationLevel::ALL) {
		TIMER("llvm_ir_optimization");

		// NOTE: This is good enough for now as I'm not sure how the passes work in llvm
		opt->driver.run(*translation_unit, opt->module_analysis);

	}
}

#undef RUN_PASS
#undef TIMER