CURRENT_DIR=$(cd "$(dirname "$0")";pwd)
./../build/db_bench --benchmarks="fillseq,readseq,readrandom"  --db="$CURRENT_DIR/../dbtest"
# --benchmarks="fillrandom" --db="../dbtest/"  --num=900000