<img src="contrib/assets/egel-black-white.svg" height="50px"/>  The Egel Language
=================

Egel is an untyped concurrent functional scripting language based on eager
combinator rewriting with a concise but remarkably powerful syntax.

Installation
------------

This interpreter is being developed on Linux/MacOS/BSD and uses libicu for 
Unicode support and fmt for formatting. 

You need to have the GNU or LLVM compiler chain for C++17, the development 
files for libicu (65.0) and fmt (8.0), and cmake (3.13) installed. 
Most package managers will provide that for you.

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

On some systems the `EGEL_PATH` environment variable needs to be set.
See the man page for further information on that.

If you don't want to do that, please note that you only need the interpreter
named `egel` and the prelude in the `include` directory for simple tasks.

Cmake generated makefiles allow for a local install with the command:

```
    make DESTDIR=~ install
```

In this case Egel components will be installed into `~/usr/local` directories
and you might refer to those components by adding the following commands
to your shell resource file, though the specific syntax may differ.

```
    export PATH=~/usr/local/bin:$PATH
    export EGEL_PATH=~/usr/local/lib/egel
```

Usage
-----

There's a [manual](https://egel-lang.github.io/egel.1.html) page you can
consult, it should be installed, or read the following short 
[introduction](http://egel.readthedocs.io/) to the interpreter on
the internet.

For a list of builtin combinators look 
[here](https://github.com/egel-lang/egel-gen/blob/main/combs.md).

Also
----

The interpreter doesn't provide command line editing, you might
want to wrap it with the command `alias egel="rlwrap egel"`.

The interpreter allocates lots of short-lived objects. If you want
a bit of extra speed, it might pay off to switch the allocator.

I use `jemalloc` on Linux by setting
``LD_PRELOAD=`jemalloc-config --libdir`/libjemalloc.so.`jemalloc-config --revision``.
