#!/bin/bash

git submodule update --init --recursive

# the vendor libraries are installed in vendor/local
mkdir local

# libicu
pushd icu/icu4c/source
chmod +x runConfigureICU configure install-sh
#./runConfigureICU MacOSX --enable-static --prefix=$(realpath "../../../local")
./runConfigureICU Linux --enable-static --prefix=$(realpath "../../../local")
#./runConfigureICU Cygwin/MSCV --enable-static --prefix=$(realpath "../../../local")
make --enable-static
make install
popd

# libffi
pushd libffi
./autogen.sh
./configure --prefix=$(realpath "../local")
make
make check
make install
popd

# lightning, depends on libtools and texinfo
pushd lightning
./bootstrap
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

