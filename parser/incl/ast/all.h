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
 Consider splitting context into pre and post production
 "{3}" produces nullptr, empty Block
	"[3]" and "(3)" work just fine
 "abs(-3) :: Float" crashes
   "abs(-2)" also crashes (the error is in the -)
 Move ast into subfolder
   Determine whether std::optional<ptr<T>> is a good pattern
   Ensure that everything works
 Ensure correct construction of generic instantiation
  "3 :: Array[Int]" -> "Array [ ast.Variable ]" in prettyPrint
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