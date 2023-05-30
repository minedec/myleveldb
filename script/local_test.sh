CURRENT_DIR=$(cd "$(dirname "$0")";pwd)
./../build/db_bench --benchmarks="fillseq,fillrandom,readrandom,readseq" --db="$CURRENT_DIR/../dbtest/"
# --benchmarks="fillseq,fillrandom,readrandom,readseq" --db="../dbtest/"