#!/bin/bash

while getopts "w:n:" opt;
do
  case $opt in
    n) client_count="$OPTARG"
      ;;
    \?) echo "Invalid option -$OPTARG" >&2
      exit 1
      ;;
    -*) echo "Option $opt needs a valid argument"
      exit 1
      ;;
  esac
done

for ((i=1;i<=client_count;i++));
do
  rm -f "/tmp/brown-bot/logs/client_${i}.log"
done