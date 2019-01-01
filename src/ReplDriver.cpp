#include "driver/ReplDriver.h"

#include "parser/ast.h"
#include "util/parser.h"

using namespace spero;
using namespace spero::compiler;

ReplDriver::ReplDriver(CompilationState& state, analysis::AllTypes& types)
	: AnalysisDriver(state, types), interpreter{ std::make_unique<llvm::Module>("seproc-repl", context) }
{}

#define TIMER(name) auto _ = state.timer(name)
void ReplDriver::addInterpreterAstTransformations() {
	// Collect all non-function definitions and move them into a new function, "jitfunc"
	// Set the `mangle` flag of jitfunc to be false and then insert into the stack

	// IDEA:
	// Take all functions as is
	// All other variable declarations will be performed as global assignments (no re-organization)?
	// Blocks can only take `ast::Statement` nodes, so any non-Statements keep in the stack
	// Anything that is not a `*Assign` node (or Interface/etc.) is wrapped inside of a jit function
	// TODO: If all nodes are `*Assign`, do ...

	if (state.failed()) {
		return;
	}

	TIMER("interpreter_transformation");

	// There's apparently no `take_if` std method
	std::deque<ptr<ast::Statement>> exprs;
	for (auto i = 0u; i != ast.size();) {
		// TODO: What would I be "losing" with this?
		if (!util::is_type<ast::Statement>(ast[i])) {
			++i;
			continue;
		}

		if (auto var_assign = util::view_as<ast::VarAssign>(ast[i])) {
			// TODO: Implement global variables. Only then can we remove this check
			if (util::is_type<ast::Function>(var_assign->expr)) {
				++i;
				continue;
			}
		}

		exprs.push_back(util::dyn_cast<ast::Statement>(std::move(ast[i])));

		auto iter = std::begin(ast);
		std::advance(iter, i);
		ast.erase(iter);
	}

	if (exprs.size() > 0) {
		auto location = exprs[0]->loc;
		auto fn = std::make_unique<ast::Function>(std::deque<ptr<ast::Argument>>{}, std::make_unique<ast::Block>(std::move(exprs), location), location);
		auto assign_name = std::make_unique<ast::AssignName>(std::make_unique<ast::BasicBinding>("jitfunc", ast::BindingType::VARIABLE, location), location);
		ast.push_back(std::make_unique<ast::VarAssign>(ast::VisibilityType::PUBLIC, std::move(assign_name), nullptr, nullptr, std::move(fn), location));
	}
}

void ReplDriver::interpretLlvm() {
	if (state.failed()) {
		return;
	}

	// Extract the "runtime" function and params from the module to ensure we run our expected code
	// NOTE: We currently only produce `jitfunc` to have type `() -> Int`
	auto jitfn = translation_unit->getFunction("jitfunc");

	// Add the next "layer" of interpreted code to the interpreter state
	// I think we still need to combine everything in a function though
	// And I'm not sure how this handles name clashes
	interpreter.addModule(std::move(translation_unit));
	translation_unit = nullptr;

	// The repl line may just define values for future usage, so the jitfn may not be created
	if (jitfn) {

		// Call the "runtime" function
		std::vector<llvm::GenericValue> params;
		auto res = interpreter.runFunction(jitfn, params);

		// Print the output
		auto& output = llvm::outs() << "result: ";

#define TYPE(x) case llvm::Type::x##TyID
		switch (jitfn->getReturnType()->getTypeID()) {
			TYPE(Integer) :
				output << res.IntVal << '\n';
			break;
			TYPE(Void) :
				TYPE(Half) :
				TYPE(Float) :
				TYPE(Double) :
				TYPE(X86_FP80) :
				TYPE(FP128) :
				TYPE(PPC_FP128) :
				TYPE(Label) :
				TYPE(Metadata) :
				TYPE(X86_MMX) :
				TYPE(Token) :
				TYPE(Function) :
				TYPE(Struct) :
				TYPE(Array) :
				TYPE(Pointer) :
				TYPE(Vector) :
			default:
				output << "Unimplemented type\n";
		}

		// Delete the `jit` function from the parent module so we always use the most recent one
		jitfn->eraseFromParent();
	}
}

bool ReplDriver::reset() {
	auto failed = state.failed();

	state.reset();
	ast.erase(std::begin(ast), std::end(ast));

	return failed;
}