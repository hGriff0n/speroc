speroc ver 0.3.4 - The reference compiler for the spero language

Project Info:

    size: 5191 sloc
    files: 9 .cpp, 17 .h, 3 .rb, 8 .rs

Libraries Used:

    PEGTL: PEG parsing
    better-enums: Extended enum support
    cxxopts: Cmd-line parsing
    x86: Interprets strings of x86 code

## Current Project Status

At the moment, I'm currently working through the recent update of the grammar to accomodate syntactical error checking (also performing some codebase updates).

Currently supported: Basic integer addition and arithmatic, Basic variables and variable usage, Variable shadowing and "revealing" in the context of scopes.
Run 'run_tests.bat' after compilation to see which systems are hooked up.

The next step is to add in mutability tracking and restriction before moving on to implementing basic aspects of the type system and function definition/calling.
