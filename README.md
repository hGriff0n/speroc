speroc v0.4.2 - The reference compiler for the spero language

# Current Project Status

Just got basic functions up and running. We are now able to call functions, passing arguments, and use their values in
computations. Recursion still has some errors with the implementation but that is probably due to the branching support.

Currently supported: Basic integer arithmatic, Basic variables and functions, Variable shadowing and scoping, Mutability Restrictions (NOTE: A basic type checking pass does exist, but is not used).

I will be taking some time to rearchitect most of the system internals to smooth over cracks that have begun to appear
and to better target the system to future avenues and opportunities (and because I've taken a month-long break). One
part of this rearchitecture will be the introduction of a new IR that final codegen will operate off of (instead of the
current direct-from-AST). The other focus will be on streamlining the storage and pass system, particularly for
accessing variable and other symbol information (ie. what we need for type and function checking).

Run 'run_tests.bat' after compilation to see which systems are hooked up.

Take a look at the full Spero documentation [here](https://github.com/hGriff0n/Spero)!

## Usage

If you have Visual Studio, this repository includes a solution file for (hopefully) easy setup and running. If not, I would greatly appreciate any pull-requests that include a custom makefile.
I just haven't been able to find the time to create it myself (I don't gain that much myself tbh).

On it's own, speroc only serves to transform Spero code into assembly files. These files are then forwarded on to clang
in order to produce the final executable. A version of clang must be installed and reachable through the command line
in order for speroc compilation (not project) to work. It is planned, though far in the future, to eventually convert
speroc to producing LLVM IR instead, and having that be forwarded on to the clang tool for improved optimization
performance.

However, I will not be doing that for now as I still want to use this project as an avenue for exploring various forms
of compiler optimizations.

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

Project Info:

    size: 4822 sloc
    files: 12 .cpp, 25 .h, 3 .rb

