speroc ver 0.3.6 - The reference compiler for the spero language

# Current Project Status

Just finished a rewrite of the grammar to allow for syntax checking and also switched over the backend to use the asmjit framework. Currently switching compilation over to using clang as the backend.

Development is focused on basic variable and type interaction issues, particularly mutability tracking (and the requisite error conditions). After that we've got basic type system characteristics and function definition/calling.

Currently supported: Basic integer addition and arithmatic, Basic variables and variable usage, Variable shadowing and "revealing" in the context of scopes.
Run 'run_tests.bat' after compilation to see which systems are hooked up.

Take a look at the full Spero documentation [here](https://github.com/hGriff0n/Spero)!

## Usage

If you have Visual Studio, this repository includes a solution file for (hopefully) easy setup and running. If not, I would greatly appreciate any pull-requests that include a custom makefile.
I just haven't been able to find the time to create it myself (I don't gain that much myself tbh).

Currently, this project forwards all created assembly files to g++ for final compilation and linking. It is planned to convert over to using clang (and the entire llvm framework) for these
stages sometime in the future. It is also planned to instead produce llvm ir, and not assembly, as the final step of this compiler (though this is farther away atm).

## Contribution

All help is appreciated. There are many issues currently in the project (and if you have ZenHub, a Kanban board), so you can just create a fork and send me a pull request.
Be sure to comment in the issue if you have any questions.

### Libraries Used:

    PEGTL: PEG parsing framework (for grammar and AST)
    better-enums: Extended enum support
    cxxopts: Cmd-line parsing
    asmjit: Assembly interaction framework (codegen)
    gcc: Current compilation and linking
    clang: Future compilation and linking stages

Project Info:

    size: 3617 sloc
    files: 9 .cpp, 15 .h, 3 .rb

