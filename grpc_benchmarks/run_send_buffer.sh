#!/bin/bash

BENCHPATH=./buffer

for ws in 256 512 1024 2048 4096 8192 16384 32768 65536 262144 2097152 8388608 33554432 134217728 268435456 536870912 1073741824 1250000000
do
      $BENCHPATH/bufservice_client $ws buffer/debug_payload_cli.out 1>> results/result-char-buf
      md5sum buffer/debug_payload_cli.out buffer/debug_payload_srv.out 1>> results/result-char-buf      
done

