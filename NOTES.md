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
