#!/bin/sh

bg=32

for i in $(seq $bg); do
    ./cachetest 1 &
done

./cachetest
