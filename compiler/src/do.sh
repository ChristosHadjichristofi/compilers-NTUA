#!/bin/sh
FILE=a.ll

if [ "$1" != "" ]; then
    make
    echo "Compiling $1..."
    ./llama < $1 || exit 1
    if [ -f "$FILE" ]; then
        llc a.ll -o a.s
        clang -lm -o a.out a.s lib.a
        # rm a.s a.ll
    fi
else
    echo "ERROR (do.sh): Provide the script name."
fi