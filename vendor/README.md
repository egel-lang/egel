# Egel external static libraries

Egel for a long while was developed on unix, where
shared libraries are unique on a distribution version
and that worked well.

On other operating systems, c++ shared libraries are 
commonly not distributed with the system. That more or
less forces the developer to build and compile used
external libraries as part of a project.

These submodules contain the external dependencies of
the egel interpreter (icu4c, ffi, fmt, and gnu
lightning).

ICU4C is a big package and, together with fmt, will
compile fine.

A bash script, `makeall.sh`, is included for convenience.
