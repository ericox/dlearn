#!/bin/bash

BENCHPATH=./buffer

# small runs
for ws in 256 512 1024 2048 4096 8192 
do
      $BENCHPATH/bufservice_client $ws 1000 >> result
done

# medium runs
for ws in 16384 32768 65536 262144
do
      $BENCHPATH/bufservice_client $ws 100 >> result
done

# large runs
for ws in 2097152 8388608 33554432 134217728 268435456
do
    $BENCHPATH/bufservice_client $ws 3 >> result
done
