#!/bin/bash

#
mkdir local

# make libicu
pushd icu/icu4c/source
chmod +x runConfigureICU configure install-sh
./runConfigureICU MacOSX --prefix=$(realpath "../../../local")
#runConfigureICU Linux
#runConfigureICU Cygwin/MSCV
make
make install
popd

# make fmt
#pushd fmt
#mkdir build
#cd build
#cmake ..
#popd

# make lightning (from a tarball since the other with a bootstrap fails)
pushd lightning-2.2.2
./configure --prefix=$(realpath "../local")
make
make check
make install
popd

