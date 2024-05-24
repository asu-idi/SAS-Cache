logpath="/data/caozhang/program/tmp/as_cache_benchmark_results/hdd/filter/2M"

# rm -rf /data/caozhang/program/tmp/existing_db_write

rm -rf /nvme/tmp/existing_db_write

../../build/db_bench --benchmarks="fillrandom" --perf_level=3 --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_size=268435456 --key_size=48 --value_size=43 --num=2000000 --db=/nvme/tmp/existing_db_write

../../build/db_bench --benchmarks="readrandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=43 --cache_size=50331648 --statistics --use_local_flash_cache=false --db=/nvme/tmp/existing_db_write --flash_cache_size=64 --num=2000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 --read_random_exp_range=0 >$logpath/nvme_0_48.txt

# ../../build/db_bench --benchmarks="readrandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=43 --cache_size=50331648 --statistics --use_local_flash_cache=true --db=/nvme/tmp/existing_db_write --flash_cache_size=96 --num=2000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 --read_random_exp_range=0 >$logpath/nvme_0_48_96.txt

# ../../build/db_bench --benchmarks="readrandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=43 --cache_size=25165824 --statistics --use_local_flash_cache=false --db=/nvme/tmp/existing_db_write --flash_cache_size=256 --num=1000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 --read_random_exp_range=15 >$logpath/nvme_15_24.txt

# ../../build/db_bench --benchmarks="readrandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=43 --cache_size=25165824 --statistics --use_local_flash_cache=false --db=/nvme/tmp/existing_db_write --flash_cache_size=256 --num=1000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 --read_random_exp_range=25 >$logpath/nvme_25_24.txt

# ../../build/db_bench --benchmarks="readrandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=43 --cache_size=25165824 --statistics --use_local_flash_cache=false --db=/nvme/tmp/existing_db_write --flash_cache_size=256 --num=1000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 --read_random_exp_range=5 >$logpath/nvme_5_24.txt

# ../../build/db_bench --benchmarks="readrandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=43 --cache_size=25165824 --statistics --use_local_flash_cache=true --db=/nvme/tmp/existing_db_write --flash_cache_size=64 --num=1000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 --read_random_exp_range=0 >$logpath/nvme_0_24_64.txt
