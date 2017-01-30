speroc - The reference compiler for the spero language

I don't know too much about compiler development so I'm at a bit of a loss in some aspects

Current Status:

    parser - largely implemented, error-prone
    ast - largely assembled with pretty-printing, error-prone
    tests - unimplemented
    analysis - unimplemented
    codegen - some organizational knowledge of PE format

Current Dev Plan:

    Augment Parser and AST with error handling
    Work on simple codegen (ie. "def main = () -> 5 + 5")
    Develop complete compiler testing framework
    Reach v1.0 by completing analysis compilation stages
    Start work on implmenting standard library

Project Info:

    size: 3603 sloc (according to Github)
    19 header files, 11 source files
    
Libraries Used:

    PEGTL: PEG parsing library
    Catch: Testing framework
    better-enums: Enum extension
    cxxopts: Cmd

Release Framework (Current expectations):

    v0.1 - Simple codegen and compilation possible
    v1.0 - Complete language specification with all analysis
    v2.0 - Language updates plus standard library and optimization stages
    v3.0 - Language updates, possible transfer to LLVM backend, maybe bootstrapped