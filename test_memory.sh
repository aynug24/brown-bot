REPEATS=1000
CLIENTS=100

echo Starting memory and file descriptor test...

for ((i = 1; i <= REPEATS; i++))
do
  for ((client = 1; client <= CLIENTS; client++))
  do
    ./bb_client -w 0 <<< "1" &
  done

  wait
done

first_mem_line=$(cat /tmp/brown-bot/logs/server.log | awk '$2 == "INC_CONN" {print $0}' | head -1 | tr -d '\0')
second_mem_line=$(tac /tmp/brown-bot/logs/server.log | awk '$2 == "INC_CONN" {print $0}' | head -1 | tr -d '\0')

echo first_mem_line
echo second_mem_line