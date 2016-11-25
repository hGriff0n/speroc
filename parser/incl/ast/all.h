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
 Implement and Improve prettyPrinting
   check the displaying of annotations once functions are added
   Consider splitting context into pre and post production
 "{3}" produces nullptr, empty Block
	"[3]" and "(3)" work just fine (something is wrong with `action<grammar::scope>`)
 "abs(-3) :: Float" crashes
   "abs(-2)" also crashes (the error is in the -)
 Ensure correct construction of generic instantiation
  "3 :: Array[Int]" -> "Array [ ast.Variable ]" in prettyPrint
 "while 3 < x x = 3" produces nullptr
   the 'x =' triggers the issue
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
 unary action doesn't run on "!false" or "-3"
   I need to double check the documentation to see where unary operators bind
 need to add error conditions and recognition into the language (that's the control part)
*/