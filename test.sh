#!/bin/bash

if [ $# -lt 1 ];then 
    numbers=3;
else
    numbers=$1;
fi;

mpic++ --prefix /usr/local/share/OpenMPI -o oets ots.cpp
dd if=/dev/random bs=1 count=$numbers of=numbers
mpirun --prefix /usr/local/share/OpenMPI -np $numbers oets
rm -f oets numbers
