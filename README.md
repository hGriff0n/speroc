speroc - The reference compiler for the spero language

I don't know too much about compiler development so I'm at a bit of a loss in some aspects

Current Status:

    parser - largely implemented, error-prone
    ast - largely assembled, error-prone
    tests - unimplemented
    analysis - unimplemented
    codegen - can produce basic ("return i") executables

Current Dev Plan:

    Work on simple codegen (ie. "def main = () -> 5 + 5")
    Augment Parser and AST with error handling
    Develop complete compiler testing framework
    Start work on implmenting standard library
    Reach v1.0 by completing analysis compilation stages

Project Info:

    size: 3603 sloc (according to Github)
    19 header files, 11 source files
    
Libraries Used:

    PEGTL: PEG parsing library
    Catch: Testing framework
    better-enums: Enum extension
    cxxopts: Cmd

Release Framework (Current expectations):

    v0.1 - Simple codegen and compilation possible with some analysis stages
    v0.2 - Type system and type inference completely setup
    v1.0 - Complete language specification with all analysis, standard library
    v2.0 - Language updates, plus optimization stages and transfer to LLVM backend
    v2.0 - Language updates, maybe bootstrapped