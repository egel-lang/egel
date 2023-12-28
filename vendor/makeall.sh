#!/bin/bash

# make libicu
pushd icu/icu4c/source
chmod +x runConfigureICU configure install-sh
./runConfigureICU MacOSX
#runConfigureICU Linux
#runConfigureICU Cygwin/MSCV
make
popd

# make fmt
pushd fmt
mkdir build
cd build
cmake ..
popd

# make lightning
pushd lightning
./bootstrap
./configure
make
make check
popd

