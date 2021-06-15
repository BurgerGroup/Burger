#!/bin/sh

killall pingpong_server
timeout=${timeout:-100}
bufsize=${bufsize:-16384}
nothreads=1

for nosessions in 1 10 100 1000 2000; do
# for nosessions in 10000; do
  sleep 5
  echo "Bufsize: $bufsize Threads: $nothreads Sessions: $nosessions"
  taskset -c 1 ../../build/release-cpp11/bin/pingpong_server 127.0.0.1 8888 $nothreads $bufsize & srvpid=$!
  sleep 1
  taskset -c 2 ../../build/release-cpp11/bin/pingpong_client 127.0.0.1 8888 $nothreads $bufsize $nosessions $timeout 1>> result.txt
  kill -9 $srvpid
done
