#!/bin/bash
for bg in 0 1 2 4 8 16 32; do
  echo $bg
  for i in $(seq $bg); do
    j=$(($i-1))
    ./cachetest 1 &
    pids[$j]=$!
  done
  sleep 15
  perf stat -e faults ./cachetest &
  wait $!
  for i in $(seq $bg); do
    j=$(($i-1))
    echo "kill background: ${pids[$j]}"
    kill -9 ${pids[$j]}
  done
  mv memaccess.log memaccess.bg.${bg}.log
  sleep 5
done

