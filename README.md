speroc ver 0.3.1 - The reference compiler for the spero language

Project Info:

    size: 4778 sloc
    files: 9 .cpp, 17 .h, 3 .rb, 8 .rs

Libraries Used:

    PEGTL: PEG parsing
    better-enums: Extended enum support
    cxxopts: Cmd-line parsing
    x86: Interprets strings of x86 code

I am currently in the process of rewritting the parser system to incorporate error checking and handling (and also improve it's capabilities). This means that some aspects of codegen are currently unconnected at the moment.

Once these changes are fully integrated, the compiler will have basic ability to "compile" basic arithmetic operations with some simple variable usage (all of these are currently unconnected though).

Once the error checking is fully integrated, and other error tasks are completed, the next step is to add in mutability tracking and restriction before moving on to implementing basic aspects of the type system and function definition/calling.
