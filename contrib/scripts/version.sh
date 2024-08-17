#!/bin/bash

if [ $# -lt 1 ]; then
  echo "usage: $0 egel.cpp" 
  exit 1
fi

major=$(sed -nE 's/#define EXECUTABLE_VERSION_MAJOR[ \t]+\"([^\"]+)\"/\1/p' "$1")
minor=$(sed -nE 's/#define EXECUTABLE_VERSION_MINOR[ \t]+\"([^\"]+)\"/\1/p' "$1")
patch=$(sed -nE 's/#define EXECUTABLE_VERSION_PATCH[ \t]+\"([^\"]+)\"/\1/p' "$1")

echo $major.$minor.$patch
