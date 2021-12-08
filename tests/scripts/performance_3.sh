#! /bin/bash

BASEDIR="."
BUILDDIR="${BASEDIR}/build"
MAX_DATA_RANGE=19

for (( version = 0; version <= 2; version++ ))
do
    for (( data_range = 4; data_range <= MAX_DATA_RANGE; data_range++ ))
    do
        $BUILDDIR/test_performance_3 -v $version -t 16 -r $data_range
        if [ $? != 0 ] 
        then
            echo "Error: execute failed"
            exit $?
        fi
    done
    echo -n -e "\n"
done