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

# make lightning (from a tarball since the other with a bootstrap fails)
pushd lightning-2.2.2
./configure
make
make check
popd

