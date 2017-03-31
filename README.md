speroc - The reference compiler for the spero language

I don't know too much about compiler development so I'm at a bit of a loss in some aspects

Current Status:

    parser - largely implemented, error-prone
    ast - largely assembled, error-prone
    tests - runner largely implemented, tests lacking
    analysis - unimplemented
    codegen - can produce basic ("return i") executables

Current Dev Plan:

    Work on simple codegen (ie. "def main = () -> 5 + 5")
    Augment Parser and AST with error handling
    Start work on implmenting standard library
    Reach v1.0 by completing analysis compilation stages

Project Info:

    size: 3827 sloc (according to Github)
    9 .h files, 5 .cpp files, 1 .rb file
    
Libraries Used:

    PEGTL: PEG parsing
    better-enums: Extended enums
    cxxopts: cmd line parsing

Release Framework (Current expectations):

    v0.1 - Simple codegen and compilation possible with some analysis stages
    v0.2 - Type system and type inference completely setup
    v1.0 - Complete initial language specification with all analysis, standard library
    v2.0 - Language updates, plus optimization stages and transfer to LLVM backend
    v2.0 - Language updates, maybe bootstrapped