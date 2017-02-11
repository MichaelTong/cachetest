#!/bin/bash

bg=16
for i in $(seq $bg); do
    j=$(($i-1))
    ./cachetest 1 &
    pids[$j]=$!
done
./cachetest

for i in $(seq $bg); do
    j=$(($i-1))
    echo "kill background: ${pids[$j]}"
    kill -9 ${pids[$j]}
done
