#!/bin/sh
FILE=a.ll

if [ "$1" != "" ]; then
    make
    echo "Compiling $1..."
    ./llama < $1 || exit 1
    if [ -f "$FILE" ]; then
        clang a.ll -o a.s -S
        clang -lm -o a.out a.ll lib.a
        # rm a.s a.ll
    fi
else
    echo "ERROR (do.sh): Provide the script name."
fi