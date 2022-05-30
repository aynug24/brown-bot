#!/bin/bash
# trap 'trap - SIGTERM && kill -- -$$' SIGINT SIGTERM EXIT

./bb_server &
server_pid=$!

echo Waiting for server to start...
sleep 1

echo
echo Running 100 client test...
./test_n_clients.sh -n 100 -w 15

echo
echo Running 100 client test again on same server...
./test_n_clients.sh -n 100 -w 20

# ./bb_zerosum &
# wait $!

kill -SIGINT "$server_pid"