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
echo "Sum of numbers in the file is zero, so the state of server after the test should be 0."
echo "Running test..."
echo
./test_n_clients.sh -n 100 -w 15

echo
echo "TEST 2:"
echo "Server shouldn't need reboot to accept more clients."
echo "So we will run the first test again with the same server."
echo "Running test..."
echo
./test_n_clients.sh -n 100 -w 20

echo
echo "TEST 3:"
echo "Server should't exhaust file descriptors for new connections, neither should its heap substantially grow if old connections are terminated."
echo "For this test we will connect to a lot of clients with almost nothing to send."
echo "Test's output are first and last log lines with new connection's file descriptor and heap pointer."
echo "Good result is when file descriptors and memory addresses are close."
echo "Running test..."
echo
./test_memory.sh

echo
echo "TEST4:"
echo "Since the server is non-blocking and not much data is sent to it, it mostly polls for clients, "
echo "and clients mostly sleep. Therefore, server should keep a batch of connections open for roughly the same time that clients sleep."
echo "For this test we will run 4, 28, 52, 76, and 100 clients with wait times of 0, 1, 2, 3, 4, and 5ms."
echo "For each batch we will display difference between mentioned periods of time (difference can be negative!)."
echo "Also, percentage of this difference in total server work time is displayed."
echo "Lower percentage means that the server is keeping up with clients and most of the time they just sleep."
echo "Running test..."
echo
./test_performance.sh 4 100 24 0 5 1

# Странное условие для теста. Файл с 1к чисел по 10 байт =>
# если в среднем останавливаемся раз в 128 байт, то спим ~80 раз.
# Если спать каждый раз минимальные 200мс, получится 16с на самый маленький тест.
# Всего по 4 минуты минимум на строчку с клиентами, а клиентов - от 1 до 100
# Итого - много часов. Если надо, можно откомменить, параметры -
# как в вызывающем цикле (clients_min, clients_max, clients_delta, аналогично
# время сна в мс.
# ./test_performance.sh 1 100 1 0 1000 100

# ./clear_server_log.sh
# ./remove_client_logs.sh -

kill -SIGINT "$server_pid"