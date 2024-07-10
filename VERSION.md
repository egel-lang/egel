VERSION
=======

A list of all major changes between versions of the interpreter.

## Pending goals

+ fix deep-recursion term printing bugs
+ replace all class preamble macros with templates
+ add import/export lists to using/namespace
+ vararg function definitions
+ move to C++20 (modules, modern syntax)
  when libfmt makes it into llvm and gcc?
+ integer move instruction in bytecode
+ generators to prelude, generator for dict
+ rewrite generators
+ vs code syntax highlighting
+ add local combinator definitions (`where`)
+ add backtick `` `f`` as a shorthand for `quote "f"`
+ dynamic dispatch
+ cleanup internals regarding combinator declarations
+ module rework and compile into local module
+ complex numbers
* clean up ref (make it serializable)
+ generate C++ from .eg file (module compilation)
+ target MSVC

## The bleeding, cutting edge

+ time and date

## v0.1.10 - egel, let's go!

+ only reduce redexes
+ simplified try/catch handling with combinators
+ added egg files, support for literate programming 
+ added a GNU lightning back-end
+ dict is now a builtin
+ revert $ precedence since $ is bitwise or
+ added a stall combinator

## v0.1.9 - great conversation, chatgpt

+ drop par for async/await
+ async tasks
+ egel namespace in c++ source
+ utilities to vm
+ more macro removal
+ egel remote procedure calls

## v0.1.8 - who cares it's slow?

+ removed internal macros with c++ methods
+ removed some utf32 bugs and code smell
+ do syntax
+ BADARGS should report the arguments
+ reintroduction of monadic min

## v0.1.7 -- fine, keep your house Stroustrup

* dis and asm should work again
* introspection
* add a proper data segment to combinators

## v0.1.6 -- rolling on

* internal cleaning
* get rid of spurious macros
+ get rid of .clone() for class::create()
* dictionary
* tinydb
* small internal tweaks to array creation
* generators library

## v0.1.5 -- advent of adventures

+ map abstract datatype
+ add `{X0,X1|XX}` pretty list syntax
+ serialization primitives

## v0.1.4 -- snakes! everywhere!

+ add a Python bridge
+ refactor code to have most (external) calls work over machine
+ static coding combinators (runtime inspection)

## v0.1.3 -- roads half built

+ swap ':' and '::', rename 'nop' to 'none'
+ added multiline strings

## v0.1.2 -- smaller is better

+ calculate and search theories

## v0.1.1 -- fear is the mind killer

+ readme/license changes
+ compile on more Linux/MacOs/BSD architectures
+ fixed critical regression LL(n) quadratic behavior

## v0.1.0 -- corrupting the neighborhood

+ MacOS (clang, arm64) and Linux (gcc, arm64)
+ man page

## v0.0.7 -- the work of the devil

+ cmake

## v0.0.6 -- rationality was a mistake

+ added reference objects
+ added monadic minus pass
+ performance (abysmal) tweaks
+ format

## v0.0.5 -- social distancing sucks

+ added lazy ops
+ added multiple commands to line parsing
+ changed default behavior to throw exceptions on bad arguments
+ processes
+ asm and dis

## v0.0.4 -- life is worth living

+ text based basic input/output and TCP transport
+ 'eval' and 'blip'

## v0.0.3 -- it may all break down yet

+ bugfix on '(1)' -length one array- results
+ switch to '#' style comments
+ regular expression support
+ small performance tweaks
+ got rid of the K combinator solution for statements

## v0.0.2 -- why bother?

+ check for overflow on simple math operators
+ changed from dot to colon syntax to free up '.' as a combinator
+ added semicolons as syntactic sugar to the language

## v0.0.1 -- heap of dung

+ CHANGED TO STANDARD PATTERN MATCHING SYNTAX
+ provisional fix to always print a dot in floats
+ fixed a scope bug in let
+ added hexadecimal numbers and binary operators
+ fixed a bug with lexical scope and try/catch blocks

## v0.0.0 -- not even wrong

This is the version where a plethora of bugs were squashed, technically
most things are in place, but there are still a large number of loose
ends.

+ symbolic rewriting
+ REPL and batch mode support
+ initial support for a prelude and basic libraries
+ experiments with mutable state and OO
+ concurrency
+ dynamic libraries
+ optional system-wide install

## v0.0 -- initial release

The initial commit. Expect lots and lots of bugs!

