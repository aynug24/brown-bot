# getting w - client wait time ms, and n - number of clients
while getopts "w:n:" opt;
do
  case $opt in
    w) wait_time="$OPTARG"
      ;;
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
    ./bb_client -w "$wait_time" <test_zerosum.txt &
done

wait

if [ "$(./bb_zerosum)" -eq 0 ]; then
    echo SUCCESS: Server sum is zero.
else
    echo FAIL: Server sum is not zero.
fi
