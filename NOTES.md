Notes about this interpreter
============================

This is a hobby project, not the next big thing. The experiment is to
see how far one can push an eager combinator rewriting implementation in
idiomatic C++. Everything about the interpreter is experimental,
and the result will be a very, and I mean very, slow interpreter.

The hope is that it will become a versatile tool for mathematics and
small domain specific language development.

My personal interest is mobile code. I hope to implement in such a way
that code can be passed around in a data center such that complex
calculations can be done with minimal traffic and start-up times.

If you are looking for a language to add to some product, I recommend
either a Lisp, Lua, or Python. 

## More about the interpreter

A number of design decision were made:

* Egel is a term rewriter that evaluates by trampolining the root
  of a tree, or more general, a directed acyclic graph (DAG).
  A garbage collector isn't implemented, Egel piggybacks on C++
  smart pointers and relies on that the runtime objects form
  a DAG at any point during evaluation.

  Egel is therefor a _very_ slow term rewriter, even slower
  than I initially expected. It's about an order of where
  I want it to be, and then still it will be very slow
  in comparison to most Lisps or Python.

  At some point I will be looking at a drop-in replacement
  for the most used smart pointer, the reference counted
  shared pointer, to get more performance.

* When C++ modules came around I jumped on them, only to find
  out that clang and gcc support were severely lacking.
  As a stopgap solution Egel is distributed with mostly 
  header-only sources. I fully intend to remove that
  stopgap when the three major compilers support modules.

* With four dependencies, and every dependency supported by
  either an os dynamic library, a user-compiled dynamic
  library, or statically linked in, there are pow(3,4) = 81
  manners of building egel.

  On foss operating systems (Linux, BSD) the four dependencies
  are usually shipped by the package manager with the exception
  of Ubuntu/Debian that doesn't ship GNU Lightning.

  * On BSD the prefered manner of shipping egel is as a
    simple application that relies on the other four 
    dependencies being installed.

  * On Linux that situation is similar with the exception
    of Ubuntu/Debian. I expect users to _compile and install_
    GNU lightning prior to installing the egel interpreter

  * On Macos and Windows, the prefered manner of shipping 
    is with the four dependencies as separate dynamic
    libraries compiled by the user.
