#!/bin/sh

percentage=3

echo "Please make sure you have done the following before run this"
echo "1. Use vmtouch to evict all file cache"
echo "2. Run mongo shell to pull all indices"
echo ""
for f in /home/sdb_mount/data/ycsb.*; do
  ./evict-cache $f $percentage
done
