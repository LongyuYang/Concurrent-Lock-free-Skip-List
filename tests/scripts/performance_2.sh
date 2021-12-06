#! /bin/bash

BASEDIR="."
BUILDDIR="${BASEDIR}/build"
MAX_THREADS=100
RESULT="./result.txt"

truncate $RESULT --size 0

for (( version = 0; version <= 2; version++ ))
do
    for (( n_threads = 1; n_threads <= $MAX_THREADS; ))
    do
        $BUILDDIR/test_performance_2 -v $version -t $n_threads -r 14 >> $RESULT
        if [ $? != 0 ];
        then
            echo "Error: execute failed"
            exit $?
        fi
        if [ "$n_threads" -le 10 ];
        then
            n_threads=`expr $n_threads + 1`
        else
            n_threads=`expr $n_threads + 10`
        fi
    done
    echo -n -e "\n" >> $RESULT
done
