#!/bin/bash

BENCHPATH=./buffer

for ws in 256 512 1024 2048 4096 8192 16384 32768 65536 262144 2097152 8388608 33554432 134217728 268435456
do
      $BENCHPATH/bufservice_client $ws cli_debug >> result
done

