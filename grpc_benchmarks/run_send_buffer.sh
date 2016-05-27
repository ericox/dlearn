#!/bin/bash

BENCH=./buffer

for ws in 256 512 1024 2048 4096 8192
do
    $BENCHPATH/bufservice_client -n $ws >> result
done


