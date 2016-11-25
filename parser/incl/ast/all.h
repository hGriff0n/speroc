#pragma once

#include "atoms.h"
#include "control.h"
#include "decor.h"
#include "literals.h"
#include "names.h"
#include "stmts.h"
#include "types.h"

/*
TODO:
 Move ast into subfolder
   Implement better pretty printing
   Determine whether std::optional<ptr<T>> is a good pattern
   Ensure that everything works
 "let x :: Int = 3" does not appear in PrettyPrinting
   It is applied correctly (same with 3 :: Int)
 some work on constructors/arguments with variables needed
 check desirability of "match x { 3 -> 3 }"
   as simple as adding a 'PatternLit' rule
 check desireability of "match x { x :: Int -> 3 }"
 global annotations shouldn't be assigned to statements
 rework PrettyPrinting
   consistent way of displaying annotations
 reduce the lookahead needed to parse import statements
 can I remove 'forward function' as a normal literal
   it's really only useful within assignment contexts anyways
 improve PrettyPrinting (compact)
   fncall argument tuple duplicates tuple
 unary action doesn't run on "!false" or "-3"
   I need to double check the documentation to see where unary operators bind
 need to add error conditions and recognition into the language (that's the control part)
*/