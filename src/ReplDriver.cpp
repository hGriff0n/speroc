#include "driver/ReplDriver.h"

#pragma warning(push, 0)
#pragma warning(disable:4996)
#include <ExecutionEngine/Interpreter/Interpreter.h>
#include <llvm/Transforms/Utils/Cloning.h>
#pragma warning(pop)

#include "parser/ast.h"
#include "util/parser.h"
#include "codegen/LlvmIrGenerator.h"

using namespace spero;
using namespace spero::compiler;

ReplDriver::ReplDriver(CompilationState& state) : AnalysisDriver{ state } {}

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
		if (!util::isType<ast::Statement>(ast[i])) {
			++i;
			continue;
		}

		if (auto var_assign = util::viewAs<ast::VarAssign>(ast[i])) {
			// TODO: Implement global variables. Only then can we remove this check
			if (util::isType<ast::Function>(var_assign->expr)) {
				++i;
				continue;
			}
		}

		exprs.push_back(util::dynCast<ast::Statement>(std::move(ast[i])));

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

	// Clone the module and construct the interpreter from it
	// NOTE: We do this because llvm::Interpreter is apparently not designed with modules in mind
	// A function in one module can not be called by code in a second module, even with the correct declarations
	// Even then, attempting to run more than "2" contexts causes the declaration approach to cause errors
	llvm::Interpreter interpreter{ llvm::CloneModule(*translation_unit) };

	// Extract the "runtime" function and params from the module to ensure we run our expected code
	// NOTE: We currently only produce `jitfunc` to have type `() -> Int`
	auto jitfn = interpreter.FindFunctionNamed("jitfunc");

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

		// Also erase it from the translation_unit as we'll never call it (and just in case)
		auto old_jitfn = translation_unit->getFunction("jitfunc");
		old_jitfn->eraseFromParent();
	}
}

bool ReplDriver::reset() {
	auto failed = state.failed();

	state.reset();
	ast.erase(std::begin(ast), std::end(ast));

	return failed;
}
