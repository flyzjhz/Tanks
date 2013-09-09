#!/bin/sh

if [ $1 = 'r' ]
then
    rm ./level.l -f
fi
gcc ./write_level.c ./level.c ./object.c
./a.out
gcc ./print_level.c ./level.c ./object.c
./a.out
