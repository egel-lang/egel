#!/bin/sh

######################################################################
# Recursive make is considered harmful! So I decided to use bash.
#
# This script copies file from a full build to system-wide locations.
#
# YOU NEED TO HAVE RUN './build'
#
# + use './install.sh' for an install.
# + use './install.sh clean' for wiping all copied files.
######################################################################

shopt -s nullglob

BIN_DIR=/usr/local/bin
LIB_DIR=/usr/local/lib
INC_DIR=/usr/local/include
INC_EGEL=$INC_DIR/egel
LIB_EGEL=$LIB_DIR/egel

ROOT=`pwd`

function copy {
    echo "copying $1 to $2"
    cp $1 $2
}

function remove {
    echo "removing $1"
    rm $1
}

function link {
    echo "link $1 with $2"
    ln -s $1 $2
}

function makedir {
    echo "making directory $1"
    mkdir $1
}

function removedir {
    echo "removing directory $1"
    rmdir $1
}

function changedir {
    echo "change directory to $ROOT/$1"
    cd $ROOT
    cd $1
}

function install {
    makedir $INC_EGEL
    makedir "$INC_EGEL/builtin"
    makedir $LIB_EGEL

    changedir src
    for filename in egel; do
        copy "$filename" "$BIN_DIR/$filename"
    done

    changedir src
    for filename in libegel.so; do
        copy "$filename" "$LIB_DIR/$filename"
    done

    changedir src
    for filename in *.hpp ; do
        copy "$filename" "$INC_EGEL/$filename"
    done

    changedir "src/builtin"
    for filename in *.hpp ; do
        copy "$filename" "$INC_EGEL/builtin/$filename"
    done

    changedir contrib/ffi/src
    for filename in ffi.hpp; do
        copy "$filename" "$INC_EGEL/$filename"
    done

    changedir include
    for filename in prelude.eg; do
        copy "$filename" "$LIB_EGEL/$filename"
    done

    changedir lib
    for filename in *.ego ; do
        copy "$filename" "$LIB_EGEL/$filename"
    done

    echo "running ldconfig on $LIB_DIR to recreate cache"
    ldconfig $LIB_DIR
}

function clean {
    changedir src
    for filename in egel; do
        remove "$BIN_DIR/$filename"
    done

    changedir src
    for filename in libegel.so; do
        remove "$LIB_DIR/$filename"
    done

    changedir src
    for filename in *.hpp; do
        remove "$INC_EGEL/$filename"
    done

    changedir "src/builtin"
    for filename in *.hpp; do
        remove "$INC_EGEL/builtin/$filename"
    done

    changedir contrib/ffi/src
    for filename in ffi.hpp; do
        remove "$INC_EGEL/$filename"
    done

    changedir include
    for filename in prelude.eg; do
        remove "$LIB_EGEL/$filename"
    done

    changedir lib
    for filename in *.ego; do
        remove "$LIB_EGEL/$filename"
    done

    #removedir $INC_EGEL
    #removedir $LIB_EGEL

}

if [ "$1" = "clean" ]; then
    clean
elif [ "$1" = "reinstall" ]; then
    clean
    install
else
    install
fi
