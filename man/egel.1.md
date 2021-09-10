EGEL(1) Version 0.1 | The Egel interpreter
==

## NAME

**egel** - an interpreted, interactive, eager-combinator language

## SYNOPSIS

`egel` [-|--interact] [-I <path>|--include <path>] [<file>]

`egel` [-I <path>|--include <path>] -e <command>

`egel` [-h|--help|-v|--version]

## DESCRIPTION

Egel is an interpreted, interactive, eager-combinator  programming language that
combines remarkable power with very clear syntax.

A short tutorial as in introduction into the language is given below.

Egel's basic power can be extended with your own modules written in C++.
Those modules are dynamically loaded.

## OPTIONS

* `-h`, `--help`:
   Prints brief usage information, may list debug options.

* `-`, `--interact`:
   Enter interactive mode unconditionally.

* `-e`, `--eval <command>`:
   Evaluate the given command.

* `-v`, `--version`:
   Prints the current version number.

* `-I`, `--include <path>`:
   Add an include path.

## TUTORIAL

Below, a short introduction to the Egel language is given.

### Expressions

Egel programs consist of expression which are evaluated eagerly.

 * Basic primitive types are integers, floats, unicode characters, and unicode strings.

   `0 1 2` , `0.0 3.14 -1.2` , `'a'` `'∀'` , or `"Hello World!"`

 * All constants compose.

   `(0 1)` is just as legal as `(cons 'a' nil)`

 * Rewriting is done with the pattern-matching abstraction, uppercase letters denote variables.

   `[ X -> X ]` , `[ (cons HEAD TAIL) -> HEAD ]`

   The abstraction consists of a number of matches, it may be variadic without penalty.

   `[ X Y -> 2 | X -> 1 | -> 0]`

 * Patterns may hold rudimentary type matches.

   `[ I::int -> "an int" | C::cons -> "a cons" ]`

 * Let expressions assign values to intermediateries.

   `let X = 1 + 2 in X * X`

 * A semicolon separates computations.

   `print (1+2); "done"`

 * Exception handling is supported, any value may be thrown and caught.

   `try 1 + throw "failure" catch [ EXC -> print EXC ]`

 * Parallell programming is achieved  through the `par` and `proc` combinators.
   A `par` starts two computations in parallel and returns a tuple of both values after both complete.

   `par [ _ -> _computation0_ ] [ _ -> _computation1_ ]` 

   The process combinator is not discussed here.

 * Formatting strings is handled with the `format` combinator, see <https://fmt.dev/>.

   `print (format "Hello {}" "world")`

### Modules

A module is a series of combinator declarations possibly encapsulated in a namespace.
All combinators are named lowercase, there is some provisional support for unicode.
Modules may import each other. The `main` combinator of the top module drives
all computation when present.

Tying it all together:

```
# A parallel fibonnaci implementation.

import "prelude.eg"

namespace Fibonnaci (
  using System

  def fib =
    [ 0 -> 0
    | 1 -> 1
    | N -> fib (N- 2) + fib (N- 1) ]

  def pfib = 
    [ 0 -> 0 
    | 1 -> 1 
    | X -> [ (F0, F1) -> F0 + F1 ]
           (par [Y -> pfib (X - 1) ] [Z-> pfib (X - 2)]) ]

)

using System

def main = Fibonnaci:pfib (3+2)
```
## EXAMPLES

There are three modes in which the interpreter is used: batch, interactive, or command mode.

In batch mode, just supply the top module with a `main` combinator.

    $ egel helloworld.eg
    Hello world!

The interpreter will start in interactive mode when invoked without a module argument.

    $ egel
    > using System
    > 1 + 1
    2

Supply a command to use `egel -e` as a simple calculator.

    $ egel -e "1+1"
    2

## FILES

The following files should be in the `EGEL_INCLUDE` directory.

 * `prelude.eg`:
   The standard Egel prelude.

 * `io.ego` `regex.ego`:
   Standard input/output, regexes dynamic libraries.

## ENVIRONMENT

 * `EGEL_INCLUDE`:
    The path to the standard include directory.

 * `EGEL_PS0`:
    The prompt given by the interpreter in interactive mode.

## BUGS

See GitHub Issues: <https://github.com/egel-lang/egel/issues>

## AUTHOR

M.C.A. (Marco) Devillers <marco.devillers@gmail.com>

## SEE ALSO

**c++(1)**
