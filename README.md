The Egel Language
=================

The Egel language is a simple untyped algebraic toy language based on 
eager combinator rewriting.

The interpreter defined here is aimed at automating small 'mathematical'
tasks.

Installation
------------

This interpreter is being developed on Linux and uses libicu for 
Unicode support. You need to have the development files for that
installed. Most Linux package managers will provide that for you.

To compile the system run the `build.sh` script.
That should give you an interpreter named `egel` in the `src` directory
and a number of dynamically loadable Egel object files in the
`include` directory.

An installation script is not provided since Linux systems tend
to have divergent opinions on where to place different parts of
a tool.

Instead, please note that you only need the interpreter named
`egel` and all files in the `include` directory if you want to
do anything useful. 
You can set the environment variable `EGEL_INCLUDE` to point 
at the latter path.

A number of example scripts are provided in the examples directory.
If you set up your system correctly, you can run any of them
with the command `egel example.eg`.

Disclaimer
----------

This is a hobby project, not the next big thing. The experiment is to
see how far one can push an eager combinator rewriting implementation in
idiomatic C++. Everything about the interpreter is experimental,
and the result will likely be a very, and I mean very, slow interpreter.
