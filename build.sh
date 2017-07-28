#!/bin/sh

######################################################################
# Recursive make is considered harmful! So I decided to use bash.
# This script either performes a make on all targets or can be used
# to wipe the directories clean.
#
# Egel doesn't provide an installation script since that it is 
# platform dependent and also different users might feel different
# on how they would like the system installed.
#
# After a build, all you need is the executable provided in the
# 'src' directory and all script or dynamic libraries in the 
# 'include' directory.
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
}

if [ "$1" = "clean" ]; then
    clean
else
    build
fi
