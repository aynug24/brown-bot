#!/bin/bash

first_col=15
data_col=16

printf "%${first_col}s" "Client count"
printf "%${data_col}s" "${s}"
echo

printf "%-17s" "78 clients:" && printf "%-${n}s" "$s" && printf "%-${n}s" "$s" && printf "%-${n}s" "$s" && printf "%-${n}s" "$s" && printf "%-${n}s" "$s" && echo

for ((client_count = 80; client_count <= 100; client_count++))
do
    for ((sleep_ms = 400; sleep_ms <= 1000; sleep_ms += 200))
    do
          for ((i=1;i<=client_count;i++));
          do
              ./bb_client -w "$sleep_ms" -l "client_${i}.log" <test_zerosum.txt &
          done

          wait

          longest_sleep_ms=$(awk '$2 == "TTL_SLP" {print $3}' /tmp/brown-bot/logs/client_*.log | sort -r | head -1)

          first_request_ms=$(cat /tmp/brown-bot/logs/server.log | awk '$2 == "INC_STRING" {print $1}' | head -1)
          last_request_ms=$(tac /tmp/brown-bot/logs/server.log | awk '$2 == "INC_STRING" {print $1}' | head -1)

          total_server_recv_requests_ms=$(last_request_ms - first_request_ms)

          exit
    done
done