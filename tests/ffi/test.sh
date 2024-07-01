#!/bin/bash

echo "compiling c library"

clang -c test.c -o test.o
clang -shared -o libtest.dylib test.o
