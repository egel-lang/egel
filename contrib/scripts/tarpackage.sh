#!/bin/bash

if [ $# -lt 2 ]; then
  echo "usage: $0 name dir [fn0..fn]"
  exit 1
fi

tarfile="$1"; shift
egeldir="$1"; shift
extras=("$@")
tardir="egel"

create_dir() {
    mkdir $1
}

remove_dir() {
    rm -rf $1
}

file_store() {
    dest="$1"; shift
    for src in $@
    do
        cp $src $dest
    done
}

create_tar() {
    tar -czf $1 $tardir
}

create_dir $tardir
create_dir $tardir/examples
file_store $tardir $egeldir/build/egel $egeldir/build/*ego
file_store $tardir $egeldir/include/*eg
file_store $tardir/examples $egeldir/examples/*
file_store $tardir $extras
create_tar $tarfile
remove_dir $tardir
