% EGEL(1) Version 0.1 | The Egel interpreter

NAME
====

**egel** — Egel, an interpreted, interactive, eager-combinator programming language

SYNOPSIS
========

| **egel** \[**-**] \[**-I** _path] \[_file_]
| **egel** \[**-I** _path] **-e** _command_
| **egel** \[**-h**|**--help**|**-v**|**--version**]

DESCRIPTION
===========

Egel is an interpreted, interactive, eager-combinator  programming language that
combines remarkable power with very clear syntax.

A short tutorial as in introduction into the language is given below.

Egel's basic power can be extended with your own modules written in C++.
Those modules are dynamically loaded.

Options
-------

-h, --help

:   Prints brief usage information. May list some extra debug options.

-, --interact

:   Enter interactive mode unconditionally.

-e, --eval _command_

:   Evaluate the given command.

-v, --version

:   Prints the current version number.

-I, --include _path_

:   Add an include path.

TUTORIAL
========

Below, a short introduction to the Egel language is given.

Expressions
-----------

Basic primitive types are integers, floats, unicode characters, and unicode strings.

:  **0 1 2** , **0.0 3.14 -1.2** , **'a' '∀'** , or **"Hello World!"**

All constants compose.

:  **(0 1)** is just as legal as **(cons 'a' nil)**

Rewriting is done with the pattern-matching abstraction, uppercase letters denote
variables. The abstraction consists of a number of matches, it may be variadic without penalty.

: **\[ X -> X ]** , **\[ (cons HEAD TAIL) -> HEAD ]**, **\[ X Y -> 2 | X -> 1 | -> 0]**

Let expressions allow to assign values to intermediateries.

: **let X = 1 + 2 in X * X**

Exception handling is supported, any value may be thrown and caught.

: **try 1 + throw "failure" catch \[ EXC -> print "aborted with: " EXC ]**

Parallellism is supported through the **par** and **proc** abstractions. A **par**
starts two computations in parallel and returns a tuple of both values after both
complete.

: **\[ (X, Y) -> X + Y ] (par \[ _ -> _computation0_ ] \[ _ -> _computation1_ ])** 

The process abstraction is not discussed here.

Formatting strings is handled with the **format** combinator, see <https://fmt.dev/>.

: **print (format "Hello {}" "world")**

Modules
-------

A module is a series of combinator declarations possibly encapsulated in a namespace.
All combinators are named lowercase, there is some provisional support for unicode.
Modules may import each other. The **main** combinator of the top module drives
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
    | X -> [ (F0, F1) -> F0 + F1 ] (par [Y -> pfib (X - 1) ] [Z-> pfib (X - 2)]) ]

)

using Fibonnaci
using System

def main = pfib 10
```

FILES
=====

The following files should be in the **EGEL_INCLUDE** directory.

*prelude.eg*

:   The standard Egel prelude.

*io.ego* *regex.ego*
:   Standard input/output, regexes.

ENVIRONMENT
===========

**EGEL_INCLUDE**

:   The default dedication if none is given.

**EGEL_PROMPT**

:   The prompt given by the interpreter in interactive mode.

BUGS
====

See GitHub Issues: <https://github.com/egel-lang/egel/issues>

AUTHOR
======

M.C.A. (Marco) Devillers <marco.devillers@gmail.com>

SEE ALSO
========

**c++(1)**
