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

 language questions:
   find ways to add restrictions to match pattern
     "match x { x :: Int -> 3 }"
	 need way to match on condition

 parser improvements:
   add in error conditions and recognition to the language
   re-enable "::" syntax for anonymous types
     changed to ":::" due to interference with type inference
     note: 'action<inf>' isn't triggered with "::"
   fix the annotation system so that global annotations aren't assigned to statements
   profile parser performance

 grammar improvements:
   add in operator precedence
     first determine operator precedence
	   scala uses the first character in the operator (https://github.com/ghik/opinionated-scala/wiki/Methods-and-operators)
   rework constructors to be more restrictive

 prettyPrinting improvements:
   differentiate Array/Block/Tuple more
   "3 :: Array[Int]" has a wierd output buffering
*/