#!/bin/sh

for f in /home/sdb_mount/data/ycsb.*; do
  ./evict-cache $f 3
done
