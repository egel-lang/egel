#!/bin/bash

# the vendor libraries are installed in vendor/local
mkdir local

# libicu
pushd icu/icu4c/source
chmod +x runConfigureICU configure install-sh
./runConfigureICU MacOSX --prefix=$(realpath "../../../local")
#runConfigureICU Linux
#runConfigureICU Cygwin/MSCV
make
make install
popd

# lightning (from a tarball since the other with a bootstrap fails)
pushd lightning-2.2.2
./configure --prefix=$(realpath "../local")
make
make check
make install
popd

# fmt
pushd fmt
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX="../../local" ..
make
make install
popd

