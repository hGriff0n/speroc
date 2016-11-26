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
 "match (3, 4) { x, y -> 4 (y) -> 3 }"
   "4 (y)" interpreted as a function call
 Ensure correct construction of generic instantiation
  "3 :: Array[Int]" -> "Array [ ast.Variable ]" in prettyPrint
 "x = 3" produces nullptr
   action<op> isn't being called
 re-enable "::" syntax for anon_types
   note "::" -> inf isn't triggered
 changed anon_type marker to ":::" due to interference with type inference (I want to change this back)
   constructor was succeeding leaving a tuple on the stack, inf rule didn't modify the tuple
 how to type a function that returns a function
   "(T) -> (T) -> T" doesn't work well
   "(T) -> ((T) -> T)" doesn't work either
 need to differentiate between Array/Tuple/Block
   bring out the details that are important to each node
 some work on constructors/arguments with variables needed
 check desirability of "match x { 3 -> 3 }"
   as simple as adding a 'PatternLit' rule
 check desireability of "match x { x :: Int -> 3 }"
 fix the annotation system so that global annottaions aren't assigned to statements
 reduce the lookahead needed to parse import statements
 can I remove 'forward function' as a normal literal
   it's really only useful within assignment contexts anyways
 need to add error conditions and recognition into the language (that's the control part)
 need to add in operator precedence
*/