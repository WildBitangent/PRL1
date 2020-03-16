#!/bin/bash

# flags="-oFast -funroll-loops -frename-registers -fno-exceptions -fomit-frame-pointer -fdelete-null-pointer-checks"
flags="-O3"
defs="-D BENCHMARK=1 -D TOPOLOGY=0"

if [ $# -eq 1 ];then 
    numbers=$1;
else
    echo "Invalid arguments."
    echo "Usage: test.sh n"
	exit 1
fi;

# gen random numbers
dd if=/dev/random bs=1 count=$numbers of=numbers status=none

# compile and run
mpic++ --prefix /usr/local/share/OpenMPI $flags $defs -o oets ots.cpp
mpirun --prefix /usr/local/share/OpenMPI -np $numbers oets

# clean
rm -f oets numbers
