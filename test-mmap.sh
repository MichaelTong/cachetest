#!/bin/bash
for bg in $(seq 32); do
    if [ ! -f imgs/random-$bg.img ]; then
        dd if=/dev/urandom of=imgs/random-$bg.img bs=1M count=1024
    fi
done

for bg in 4; do
    echo $bg
    echo 3 | sudo tee /proc/sys/vm/drop_caches
    for i in $(seq $bg); do
        j=$(($i-1))
        ./cachetest-mmap $i &
        pids[$j]=$!
    done
    sleep 2
    /usr/bin/time -v ./cachetest-mmap 0 &
    wait $!
    for i in $(seq $bg); do
        j=$(($i-1))
        echo "kill background: ${pids[$j]}"
        kill -9 ${pids[$j]}
    done
    mv outputs/memaccess.log outputs/memaccess.bg.${bg}.log
    sleep 5
done

