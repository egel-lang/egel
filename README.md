<img src="contrib/assets/egel-black-white.svg" height="50px"/>  The Egel Language
=================

Egel is an untyped concurrent functional scripting language based on eager
combinator rewriting with a concise but remarkably powerful syntax.

Installation
------------

This interpreter is being developed on Linux/MacOS/BSD and uses icu4c for 
Unicode support, fmt for formatting, ffi as a foreign function interface,
and GNU lightning as a ahead-of-time backend. 

The interpreter can be compiled with a current C++ compiler and employs
cmake to build.

There are roughly two manners in which operating systems and package
managers provide C++ libraries.

1. The open source model (various Unixes and BSDs) where C++ libraries are
   compiled and dissimated by the operating system distributor. Use your
   package manager to install libicu, fmt, and GNU lightning.

2. The vendor based model (MacOS and Windows) where C++ libraries are 
   usually not provided since they are brittle to link against, and where
   one usually compiles these libraries from scratch and statically links
   them in or distributes them with the application. 
   Links to the vendors are provided as git submodules in the
   vendor directory and, you're on your own here, you'll need to download
   and compile those libraries. There's a separate README.md in the vendor
   directory that should help somewhat.

CMake files are provided for both models, select the one you want to use
in cmake with 
and rename either to `CMakeLists.txt`.

After that the interpreter is made with `cmake` in the standard manner. Run
the following commands on a Linux system.

```
    mkdir build
    cd build
    cmake ..
    make
```

note: for older GCC you sometimes need to uncomment the
`stdc++fs` rule.

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
