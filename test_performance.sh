#!/bin/bash

FIRST_COL=15
DATA_COL=16

CLIENT_MIN=4
CLIENT_MAX=100
CLIENT_DELTA=24

SLEEP_MIN=0
SLEEP_MAX=1000
SLEEP_DELTA=200


./clear_server_log.sh
./remove_client_logs.sh -n 100


printf "%${FIRST_COL}s" "Clients\Sleep"
for ((sleep_ms = SLEEP_MIN; sleep_ms <= SLEEP_MAX; sleep_ms += SLEEP_DELTA))
do
  printf "%${DATA_COL}s" "$( bc <<< "scale=1;${sleep_ms}/1000" )s"
done
echo


for ((client_count = CLIENT_MIN; client_count <= CLIENT_MAX; client_count += CLIENT_DELTA))
do
    printf "%${FIRST_COL}s" "${client_count} clients:"

    for ((sleep_ms = SLEEP_MIN; sleep_ms <= SLEEP_MAX; sleep_ms += SLEEP_DELTA))
    do
          for ((i=1;i<=client_count;i++))
          do
              ./bb_client -w "$sleep_ms" -l "client_${i}.log" <test_zerosum.txt &
          done

          wait

          longest_sleep_ms=$(awk '$2 == "TTL_SLP" {print $3}' /tmp/brown-bot/logs/client_*.log | sort -r | head -1)

          first_request_ms=$(cat /tmp/brown-bot/logs/server.log | awk '$2 == "INC_STRING" {print $1}' | head -1 | tr -d '\0')
          last_request_ms=$(tac /tmp/brown-bot/logs/server.log | awk '$2 == "INC_STRING" {print $1}' | head -1 | tr -d '\0')

          total_server_recv_requests_ms=$(( last_request_ms - first_request_ms ))

          efficiency_metric_ms=$(( total_server_recv_requests_ms - longest_sleep_ms ))

          printf "%${DATA_COL}s" "${efficiency_metric_ms}ms ($(( 100 * efficiency_metric_ms / total_server_recv_requests_ms ))%)"

          # echo "${longest_sleep_ms} sleep, ms"
          # echo "${total_server_recv_requests_ms} server, ms"

          ./clear_server_log.sh
          ./remove_client_logs.sh -n "${client_count}"

          sleep 10
    done

    echo
done