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
 "foo :: (Int)" crashes
   stack at fneps rule: nullptr Variable{"foo"} nullptr nullptr Binding{"Int"}
     everything after "foo" should not be there
 "match (3, 4) { x, y -> 4 (y) -> 3 }"
   "4 (y)" interpreted as a function call
 Ensure correct construction of generic instantiation
  "3 :: Array[Int]" -> "Array [ ast.Variable ]" in prettyPrint
 "--3" crashes
 "x = 3" produces nullptr
   action<op> isn't being called
 how to type a function that returns a function
   "(T) -> (T) -> T" doesn't work well
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
 tuple printing on size=0 can be improved
*/