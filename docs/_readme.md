# Current Project Status

Just finished a rewrite of the grammar to allow for syntax checking and also switched over the backend to use the asmjit framework. Currently switching compilation over to using clang as the backend.

Development is focused on basic variable and type interaction issues, particularly mutability tracking (and the requisite error conditions). After that we've got basic type system characteristics and function definition/calling.

Currently supported: Basic integer addition and arithmatic, Basic variables and variable usage, Variable shadowing and "revealing" in the context of scopes.
Run 'run_tests.bat' after compilation to see which systems are hooked up.

Take a look at the full Spero documentation [here](https://github.com/hGriff0n/Spero)!

## Usage

If you have Visual Studio, this repository includes a solution file for (hopefully) easy setup and running. If not, I would greatly appreciate any pull-requests that include a custom makefile.
I just haven't been able to find the time to create it myself (I don't gain that much myself tbh).

On it's own, speroc only serves to transform Spero code into assembly files. These files are then forwarded on to clang in order to produce the final executable. A version of clang must be
installed and reachable through the command line in order for speroc compilation (not project) to work. It is planned, though far in the future, to eventually convert speroc to producing
LLVM IR instead, and having that be forwarded on to the clang tool for improved optimization performance.

## Contribution

All help is appreciated. There are many issues currently in the project (and if you have ZenHub, a Kanban board), so you can just create a fork and send me a pull request.
Be sure to comment in the issue if you have any questions.

### Libraries Used:

    PEGTL: PEG parsing framework (for grammar and AST)
    better-enums: Extended enum support
    cxxopts: Cmd-line parsing
	spdlog: Logging and error reporting
    asmjit: Assembly interaction framework (codegen)
	  NOTE: Currently using a custom fork that adds move semantics
    clang: Executable production and linking
    boost:
      flyweight - string interning engine
