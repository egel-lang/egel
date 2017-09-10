#!/bin/sh

######################################################################
# Recursive make is considered harmful! So I decided to use bash.
#
# This script either performes a 'make O3' on all targets or can be
# used to wipe the directories clean.
#
# Egel doesn't provide an installation script since that it is 
# platform dependent and different users might feel different on how
# they would like the interpreter to be installed.
#
# After a build, all you need is the executable provided in the
# 'src' directory and all scripts or dynamic libraries in the 
# 'include' directory.
#
# + use './build.sh' for a build.
# + use './build.sh clean' for wiping all generated files
######################################################################

function build {
    cd src
    make O3
    cd ..

    cd include/math
    make O3
    cd ../..

    cd include/io
    make O3
    cd ../..

    cd include/util
    make O3
    cd ../..

    cd include/par
    make O3
    cd ../..

    cd include/random
    make O3
    cd ../..

    cd include/string
    make O3
    cd ../..
}

function clean {
    cd src
    make clean
    cd ..

    cd include/math
    make clean
    cd ../..

    cd include/io
    make clean
    cd ../..

    cd include/util
    make clean
    cd ../..

    cd include/par
    make clean
    cd ../..

    cd include/random
    make clean
    cd ../..

    cd include/string
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
