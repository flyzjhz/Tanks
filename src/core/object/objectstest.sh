#!/bin/sh

if [ $1 = 'r' ]
then
    rm ./object.l -f
fi
gcc ./write_object.c ./object.c
./a.out
gcc ./print_all_object.c ./object.c
./a.out
