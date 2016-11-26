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
 "abs(-3)" crashes
  "-3.abs" doesn't crash
 "foo :: (Int, Int) -> Int" crashes
   "foo :: (Int)" also crashes
 "match (3, 4) { x, y -> 4 (y) -> 3 }"
   "4 (y)" interpreted as a function call
 "def Foo = (x :: Int) { }" crashes
   "def Foo = None {}" and "def Foo = Some(Int) {}" don't crash
 Ensure correct construction of generic instantiation
  "3 :: Array[Int]" -> "Array [ ast.Variable ]" in prettyPrint
 "x = 3" produces nullptr
   action<op> isn't being called
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