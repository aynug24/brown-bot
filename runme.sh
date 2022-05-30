#!/bin/bash
# trap 'trap - SIGTERM && kill -- -$$' SIGINT SIGTERM EXIT

killall bb_server
killall bb_client

./clear_server_log.sh
./remove_client_logs.sh

./bb_server -l server.log &
server_pid=$!

echo Waiting for server to start...
sleep 1
echo Server started!

echo
echo Running 100 client test...
./test_n_clients.sh -n 100 -w 15

echo
echo Running 100 client test again on same server...
./test_n_clients.sh -n 100 -w 20

echo
./test_performance.sh

# ./clear_server_log.sh
# ./remove_client_logs.sh -

kill -SIGINT "$server_pid"