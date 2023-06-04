cd perf_pm

perf record -e cpu-clock -F 50 -g --call-graph=dwarf ./../../build/db_bench --benchmarks="fillrandom" --db="/mnt/pmem0/dbtest"
mv perf.data perf_pm_fillseq.data

sudo perf script -i perf_pm_fillseq.data > out_perf_pm_fillseq.perf

../../../FlameGraph/stackcollapse-perf.pl out_perf_pm_fillseq.perf > out_perf_pm_fillseq.folded

../../../FlameGraph/flamegraph.pl out_perf_pm_fillseq.folded > perf_pm_fillseq.svg

cd ../

