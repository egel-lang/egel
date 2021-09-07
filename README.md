<img src="contrib/assets/egel-logo.svg" height="50px"/>  The Egel Language
=================

The Egel language is a simple untyped algebraic toy language based on 
eager combinator rewriting.

The interpreter defined here is aimed at automating small 'mathematical'
tasks.

Installation
------------

This interpreter is being developed on Linux/MacOS and uses libicu for 
Unicode support and fmt for formatting. 

You need to have the GNU or LLVM compiler chain, the development 
files for libicu and fmt, and cmake installed. 
Most Linux package managers will provide that for you.

This interpreter is made with `cmake` in the standard manner. Run
the following commands on a Linux system.

```
    mkdir build
    cd build
    cmake ..
    make
```

note: on MacOS you need to provide the location of what you want
to link against. Since most people will be using brew, use this

```
    cmake .. -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/icu4c
```

That should give you an interpreter named `egel`
and a number of dynamically loadable Egel object files in the
`build` directory.

For a system-wide install run `make install` after a build
as root.

If you don't want to do that, please note that you only need the interpreter
named `egel` and all files in the `include` directory for simple tasks.
You can set the environment variable `EGEL_INCLUDE` to point 
at the latter path.

Usage
-----

The command `egel` will start the interpreter in interactive mode.
You will be greeted with a prompt. Note that the interpreter comes
in a 'clean state' mode, your first command will probably be
`using System` to get direct access to Egel's builtin primitives,
like addition, printing, etcetera.

The interpreter doesn't provide command line editing, you might
want to wrap it with the command `alias egel="rlwrap egel"`.

A number of example scripts are provided in the examples directory.
If you set up your system correctly, you can run any of them
with the command `egel example.eg`.

The interpreter allocates lots of short-lived objects. If you want
a bit of extra speed, it might pay off to switch the allocator to
`jemalloc`. I use 
``LD_PRELOAD=`jemalloc-config --libdir`/libjemalloc.so.`jemalloc-config --revision``.

Disclaimer
----------

This is a hobby project, not the next big thing. The experiment is to
see how far one can push an eager combinator rewriting implementation in
idiomatic C++. Everything about the interpreter is experimental,
and the result will likely be a very, and I mean very, slow interpreter.

This is the interpreter version v0.0, alpha, where I am in the process
of simplifying the code and stamping out bugs.
