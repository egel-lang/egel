EGEL(1) Version 0.1 | The Egel interpreter
==

## NAME

**egel** - an untyped functional scripting language

## SYNOPSIS

`egel` [-|--interact] [-I <path>|--include <path>] [<file>]

`egel` [-I <path>|--include <path>] -e <command>

`egel` [-h|--help|-v|--version]

## DESCRIPTION

Egel is an untyped concurrent functional scripting language based on eager combinator
rewriting with a concise but remarkably powerful syntax.

Egel's basic functionality can be extended with your own modules written in C++.
Those modules are dynamically loaded.

## OPTIONS

* `-h`, `--help`:
   Prints brief usage information, may list debug options.

* `-v`, `--version`:
   Prints the current version number.

* `-`, `--interact`:
   Enter interactive mode unconditionally.

* `-e`, `--eval <command>`:
   Evaluate the given command.

* `-I`, `--include <path>`:
   Add an include path.

## TUTORIAL

Egel is an expression language and the interpreter a symbolic evaluator.

### Expressions

Egel code consist of expression which are evaluated eagerly.

 * Basic primitives are integers, floats, unicode characters, and unicode strings.

   `0 1 2` , `0.0 3.14 -1.2` , `'a'` `'∀'` , or `"Hello World!"`

 * All constants compose.

   `(0 1)` is just as legal as `(cons 'a' nil)`

 * Rewriting is done with the pattern-matching anonymous abstraction, uppercase letters denote variables.

   `[ X -> X ]` , `[ (cons HEAD TAIL) -> HEAD ]`

   The abstraction consists of a number of matches, it may be variadic without penalty.

   `[ X Y -> 2 | X -> 1 | -> 0]`

   A backslash `\` starts a lambda abstraction.

   `\(cons X XX) -> X`

 * Patterns may match against tags.

   `[ I:int -> "an int" | C:cons -> "a cons" ]`

 * Let expressions assign values to intermediaries.

   `let X = 1 + 2 in X * X`

 * A semicolon separates computations.

   `print (1+2); "done"`

 * Exception handling is supported, any value may be thrown and caught.

   `try 1 + throw "failure" catch [ EXC -> print EXC ]`

 * The do notation composes chains of transformations.

   `(do ((+) 1) |> foldl (*) 1) (from_to 0 4)`

 * Parallell programming is achieved  through the `async/await` combinators.
   Asynchronous programs start threads that return future objects.

   `let F = async [ _ -> fib 35 ] in await F` 

 * Formatting strings is handled with the `format` combinator, see <https://fmt.dev/>.

   `print (format "Hello {}" "world")`

 * The interpreter implements a term rewriter though has mutable references.
   Cycles won't be garbage collected.

   `let X = ref 0 in set_ref X 1; get_ref X`

### Modules

A module is a series of combinator declarations possibly encapsulated in a namespace.
All combinators are named lowercase, there is some provisional support for unicode.
Modules may import each other. The `main` combinator of the top module drives
all computation when present.

Tying it all together:

```
# A Fibonacci implementation.

import "prelude.eg"

namespace Fibonacci (
  using System

  def fib =
    [ 0 -> 0
    | 1 -> 1
    | N -> fib (N- 2) + fib (N- 1) ]

)

using System

def main = Fibonacci::fib (3+2)
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

Supply a command to use `egel -e` as a simple calculator. Double semicolons are separators.

    $ egel fib.eg -e "using Fibonacci;; fib 3"
    5

## FILES

The following files should be in the `EGEL_PATH` directory.

 * `prelude.eg` `calculate.eg` `search.eg`:
   The standard Egel prelude and additional scripts.

 * `os.ego` `fs.ego` `regex.ego`:
   input/output, filesystem, regexes dynamic libraries.

## ENVIRONMENT

 * `EGEL_PATH`:
    The path to the standard include directory.

 * `EGEL_PS0`:
    The prompt given by the interpreter in interactive mode.

## BUGS

See GitHub Issues: <https://github.com/egel-lang/egel/issues>

## AUTHOR

MIT License (c) 2017 M.C.A. (Marco) Devillers <marco.devillers@gmail.com>

## SEE ALSO

**c++(1)**
