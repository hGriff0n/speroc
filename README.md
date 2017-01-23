speroc - The reference compiler for the spero language

I don't know too much about compiler development so I'm at a bit of a loss in some aspects

Current Status:

    parser - largely implemented, error-prone
    ast - largely assembled with pretty-printing, error-prone
    tests - unimplemented
    analysis - unimplemented
    codegen - unimplemented

Current Dev Plan:

    Augment Parser and AST with error handling
    Work on simple codegen code (ie. "def main = () -> 5 + 5")

Project Info:

    size: 3200 sloc (according to Github)
    17 header files, 11 source files
    
Libraries Used:

    PEGTL: PEG parsing library
    Catch: Testing framework
    better-enums: Enum extension
    cxxopts: Cmd