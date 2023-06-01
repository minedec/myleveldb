cd perf_local

perf record -e cpu-clock -g ./../../build/db_bench --benchmarks="fillseq" --db="/home/jjd/dbtest"
mv perf.data perf_local_fillseq.data
perf record -e cpu-clock -g ./../../build/db_bench --benchmarks="fillrandom" --db="/home/jjd/dbtest"
mv perf.data perf_local_fillrandom.data
perf record -e cpu-clock -g ./../../build/db_bench --benchmarks="fillrandom,readseq" --db="/home/jjd/dbtest"
mv perf.data perf_local_frreadseq.data
perf record -e cpu-clock -g ./../../build/db_bench --benchmarks="fillrandom,readrandom" --db="/home/jjd/dbtest"
mv perf.data perf_local_frreadrandom.data

sudo perf script -i perf_local_fillseq.data > out_perf_local_fillseq.perf
sudo perf script -i perf_local_fillrandom.data > out_perf_local_fillrandom.perf
sudo perf script -i perf_local_frreadseq.data > out_perf_local_frreadseq.perf
sudo perf script -i perf_local_frreadrandom.data > out_perf_local_frreadrandom.perf

../../../FlameGraph/stackcollapse-perf.pl out_perf_local_fillseq.perf > out_perf_local_fillseq.folded
../../../FlameGraph/stackcollapse-perf.pl out_perf_local_fillrandom.perf > out_perf_local_fillrandom.folded
../../../FlameGraph/stackcollapse-perf.pl out_perf_local_frreadseq.perf > out_perf_local_frreadseq.folded
../../../FlameGraph/stackcollapse-perf.pl out_perf_local_frreadrandom.perf > out_perf_local_frreadrandom.folded

../../../FlameGraph/flamegraph.pl out_perf_local_fillseq.folded > perf_local_fillseq.svg
../../../FlameGraph/flamegraph.pl out_perf_local_fillrandom.folded > perf_local_fillrandom.svg
../../../FlameGraph/flamegraph.pl out_perf_local_frreadseq.folded > perf_local_frreadseq.svg
../../../FlameGraph/flamegraph.pl out_perf_local_frreadrandom.folded > perf_local_frreadrandom.svg

cd ../

