<img src="contrib/assets/egel-logo.svg" height="50px"/>  The Egel Language
=================

The Egel language is a simple untyped algebraic toy language based on 
eager combinator rewriting.

The interpreter defined here is aimed at automating small 'mathematical'
tasks.

Installation
------------

This interpreter is being developed on Linux and uses libicu for 
Unicode support. You need to have GCC/g++, the GNU compiler chain,
and the development files for libicu
installed. Most Linux package managers will provide that for you.

To compile the system run the `build.sh` script.
That should give you an interpreter named `egel` in the `src` directory
and a number of dynamically loadable Egel object files in the
`lib` directory.

For a system-wide install run the `install.sh` script after a build
as root. On a Fedora system..

If you don't want to do that, please note that you only need the interpreter
named `egel` and all files in the `include` directory for simple tasks.
You can set the environment variable `EGEL_INCLUDE` to point 
at the latter path.

A number of example scripts are provided in the examples directory.
If you set up your system correctly, you can run any of them
with the command `egel example.eg`.

Egel doesn't provide command line editing. For that, you might
want to wrap it in `rlwrap` with the command 
`alias egel=rlwrap egel`.

Disclaimer
----------

This is a hobby project, not the next big thing. The experiment is to
see how far one can push an eager combinator rewriting implementation in
idiomatic C++. Everything about the interpreter is experimental,
and the result will likely be a very, and I mean very, slow interpreter.

This is the interpreter version v0.0, alpha, where I am in the process
of simplifying the code and stamping out bugs.
