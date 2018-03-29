#!/bin/sh

######################################################################
# Recursive make is considered harmful! So I decided to use bash.
#
# This script either performes a 'gmake O3' on all targets or can be
# used to wipe the directories clean.
#
# After a build, all you need is the executable provided in the
# 'src' directory and all scripts or dynamic includeraries in the 
# 'include' directory.
#
# + use './build.sh' for a build.
# + use './build.sh clean' for wiping all generated files
######################################################################

function build {
    cd src
    gmake O3
    gmake shared
    gmake archive
    cd ..

    cd lib/io
    gmake O3
    cd ../..

    cd lib/random
    gmake O3
    cd ../..

    cd lib/regex
    gmake O3
    cd ../..
}

function clean {
    cd src
    gmake clean
    cd ..

    cd lib/io
    gmake clean
    cd ../..

    cd lib/random
    gmake clean
    cd ../..

    cd lib/regex
    gmake clean
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
