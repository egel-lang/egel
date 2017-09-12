#!/bin/sh

######################################################################
# Recursive make is considered harmful! So I decided to use bash.
#
# This script either performes a 'make O3' on all targets or can be
# used to wipe the directories clean.
#
# After a build, all you need is the executable provided in the
# 'src' directory and all scripts or dynamic libraries in the 
# 'lib' directory.
#
# + use './build.sh' for a build.
# + use './build.sh clean' for wiping all generated files
######################################################################

function build {
    cd src
    make O3
    make shared
    make archive
    cd ..

    cd lib/math
    make O3
    cd ../..

    cd lib/io
    make O3
    cd ../..

    cd lib/util
    make O3
    cd ../..

    cd lib/par
    make O3
    cd ../..

    cd lib/random
    make O3
    cd ../..

    cd lib/string
    make O3
    cd ../..
}

function clean {
    cd src
    make clean
    cd ..

    cd lib/math
    make clean
    cd ../..

    cd lib/io
    make clean
    cd ../..

    cd lib/util
    make clean
    cd ../..

    cd lib/par
    make clean
    cd ../..

    cd lib/random
    make clean
    cd ../..

    cd lib/string
    make clean
    cd ../..
}

if [ "$1" = "clean" ]; then
    clean
elif [ "$1" = "rebuild" ]; then
    clean
    build
else
    build
fi
