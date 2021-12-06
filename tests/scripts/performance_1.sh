#! /bin/bash

BASEDIR="."
BUILDDIR="${BASEDIR}/build"
MAX_THREADS=8

for (( n_threads = 1; n_threads <= $MAX_THREADS; n_threads <<= 1 ))
do
    echo "==========================================="
    for (( version = 0; version <= 2; version++ ))
    do
        $BUILDDIR/test_performance_1 -v $version -t $n_threads
        if [ $? != 0 ] 
        then
            echo "Error: execute failed"
            exit $?
        fi
    done
    echo "==========================================="
done
    