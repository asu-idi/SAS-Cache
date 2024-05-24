rm -rf /tmp/existing_db_cachelib

./build/db_bench --benchmarks="fillseq,stats" --perf_level=2 --key_size=8 --value_size=64 --statistics --db=/tmp/existing_db_cachelib --num=1000000

./build/db_bench --benchmarks="readrandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=8 --value_size=64 --statistics --use_existing_db=true --db=/tmp/existing_db_cachelib --num=1000000 --threads=32 --read_random_exp_range=5 --use_local_flash_cache=true --flash_cache_size=5120 > trash.txt