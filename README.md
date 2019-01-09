speroc v0.5.0 - The reference compiler for the spero language

# Current Project Status

Just finished porting the internal backend stages over to producing llvm ir. A big benefit of this work is that we automatically get access to a lot of optimizations and target architectures. This also
vastly simplifies the problem of backend code generation, as we now only need to target 1 language that we know produces correct assembly.

As of now, the compiler supports basic function and variable usage. We can call and declare functions, passing arguments, and using their values in computations - recursion fully supported. Variables can
be declared with full shadowing support (though the corner cases are not well tested) and mutability restrictions. Basic if-then-else branching is supported, but no other codeflow constructs have been implemented.
More advanced types then `Int` and `Bool` are not fully supported and will likely cause llvm assertion errors when using. Functions also must be declared before usage, even in the global scope

Next work will be on implementing a type inference system and "liberalizing" the amount of type work that may be done internally (such as allowing for defining custom types).

Take a look at the full Spero documentation [here](https://github.com/hGriff0n/Spero)!

## Usage

If you have Visual Studio, this repository includes a solution file for (hopefully) easy setup and running. If not, I would greatly appreciate any pull-requests that include a custom makefile.
I just haven't been able to find the time to create it myself (as I use Visual Studio for heavy C++ dev-work).

On it's own, speroc only serves to transform Spero code into llvm-ir files. These files are then forwarded on to clang in order to produce the final executable. A version of clang must be
installed and reachable through the command line in order for speroc compilation (not project) to work.

## Contribution

All help is appreciated. There are many issues currently in the project (and if you have ZenHub, a Kanban board), so you can just create a fork and send me a pull request.
Be sure to comment in the issue if you have any questions.

### Libraries Used:

    PEGTL: PEG parsing framework (for grammar and AST)
    better-enums: Extended enum support
    cxxopts: Cmd-line parsing
	spdlog: Logging and error reporting
    llvm: Produce llvm ir, ir optimization passes, final compilation (clang)
    boost:
      flyweight - string interning engine

Project Info:

    size: 4948 sloc
    files: 13 .cpp, 24 .h, 3 .rb

