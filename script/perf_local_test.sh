cd perf_local

perf record -e cpu-clock -F 60 -g --call-graph=dwarf ./../../build/db_bench --benchmarks="fillseq" --db="/home/jjd/dbtest"
mv perf.data perf_local_fillseq.data

sudo perf script -i perf_local_fillseq.data > out_perf_local_fillseq.perf

../../../FlameGraph/stackcollapse-perf.pl out_perf_local_fillseq.perf > out_perf_local_fillseq.folded

../../../FlameGraph/flamegraph.pl out_perf_local_fillseq.folded > perf_local_fillseq.svg

cd ../

