#!/bin/sh

if [ "$1" != "" ]; then
    make
    echo "Compiling $1..."
    ./llama < $1 > a.ll || exit 1
    llc a.ll -o a.s
    clang -o a.out a.s lib.a
    clear
    ./a.out
    # rm a.out a.s a.ll
else
    echo "ERROR (do.sh): Provide the script name."
fi