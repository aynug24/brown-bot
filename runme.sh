#!/bin/bash
trap 'trap - SIGTERM && kill -- -$$' SIGINT SIGTERM EXIT

rm -f result.txt
exec > >(tee -ia result.txt)  # tee all output to result.txt

echo "Welcome!"
echo "This is a socket client-server application."
echo "Server receives lines of numbers from clients and stores sum of all received numbers."
echo "I tried to make the app work non-blockingly. Let's test it!"
echo

killall -q bb_server
killall -q bb_client

./clear_server_log.sh
./remove_client_logs.sh

./bb_server -l server.log &
server_pid=$!

echo "Waiting for server to start..."
sleep 1
echo "Server started!"

echo
echo "TEST 1:"
echo "100 clients connect to the server and send contents of file test_zerosum.txt."
echo "Sum of numbers in this file is zero, so the state of server after the test should be 0"
echo
./test_n_clients.sh -n 100 -w 15

echo
echo "TEST 2:"
echo "Server shouldn't need reboot to accept more clients."
echo "So we will run the first test again with the same server."
echo
./test_n_clients.sh -n 100 -w 20

echo
echo "TEST 3:"
echo "Server should't exhaust file descriptors for new connections, neither should its heap substuntially grow if old connections are terminated."
echo "For this test we will connect with a lot of clients with almost nothing to send."
echo "Test's output are first and last log lines with new connection's file descriptor and heap pointer."
./test_memory.sh

echo
echo "TEST4:"
echo ""
./test_performance.sh

# ./clear_server_log.sh
# ./remove_client_logs.sh -

kill -SIGINT "$server_pid"