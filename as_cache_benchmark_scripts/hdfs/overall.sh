logpath="/data/caozhang/program/tmp/as_cache_benchmark_results/hdd/overall/rw90"


rm -rf /data/caozhang/program/tmp/existing_db_write 

../../build/db_bench --benchmarks="readrandomwriterandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=8 --value_size=1024 --cache_size=100663296 --statistics --use_local_flash_cache=true --db=/data/caozhang/program/tmp/existing_db_write --flash_cache_size=256 --num=5000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 --readwritepercent=90 >$logpath/96_256.txt

rm -rf /data/caozhang/program/tmp/existing_db_write 

../../build/db_bench --benchmarks="readrandomwriterandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=8 --value_size=1024 --cache_size=100663296 --statistics --use_local_flash_cache=true --db=/data/caozhang/program/tmp/existing_db_write --flash_cache_size=256 --num=5000000 --use_flash_cache_filter=true --enable_flash_admission_list=false --dram_cache_size=8 --readwritepercent=90 >$logpath/96_256_filter.txt

rm -rf /data/caozhang/program/tmp/existing_db_write 

../../build/db_bench --benchmarks="readrandomwriterandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=8 --value_size=1024 --cache_size=100663296 --statistics --use_local_flash_cache=true --db=/data/caozhang/program/tmp/existing_db_write --flash_cache_size=256 --num=5000000 --use_flash_cache_filter=true --enable_flash_admission_list=true --dram_cache_size=8 --readwritepercent=90 >$logpath/96_256_filter_list.txt

rm -rf /data/caozhang/program/tmp/existing_db_write 

../../build/db_bench --benchmarks="readrandomwriterandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=8 --value_size=1024 --cache_size=100663296 --statistics --use_local_flash_cache=true --db=/data/caozhang/program/tmp/existing_db_write --flash_cache_size=256 --num=5000000 --use_flash_cache_filter=true --enable_flash_admission_list=true --dram_cache_size=8 --readwritepercent=90 --enable_flash_evict_blocks=true --enable_flash_prefetch_files=true >$logpath/96_256_all.txt
# rm -rf /data/caozhang/program/tmp/existing_db_write 

# ../../build/db_bench --benchmarks="readrandomwriterandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=8 --value_size=1024 --cache_size=134217728 --statistics --use_local_flash_cache=true --db=/data/caozhang/program/tmp/existing_db_write --flash_cache_size=1024 --num=10000000 --use_flash_cache_filter=true --enable_flash_admission_list=false --dram_cache_size=8 --readwritepercent=90 >$logpath/128_1024_filter.txt

# rm -rf /data/caozhang/program/tmp/existing_db_write 

# ../../build/db_bench --benchmarks="readrandomwriterandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=8 --value_size=1024 --cache_size=134217728 --statistics --use_flash_cache_filter=true --use_local_flash_cache=true --db=/data/caozhang/program/tmp/existing_db_write --flash_cache_size=1024 --num=10000000 --use_flash_cache_filter=false --enable_flash_admission_list=true --dram_cache_size=8 --readwritepercent=90 >$logpath/128_1024_list.txt


# rm -rf /data/caozhang/program/tmp/existing_db_write 

# ../../build/db_bench --benchmarks="readrandomwriterandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=8 --value_size=1024 --cache_size=134217728 --statistics --use_flash_cache_filter=true --use_local_flash_cache=true --db=/data/caozhang/program/tmp/existing_db_write --flash_cache_size=1024 --num=10000000 --use_flash_cache_filter=true --enable_flash_admission_list=true --enable_flash_evict_blocks=false --enable_flash_prefetch_files=false --dram_cache_size=8 --readwritepercent=90 >$logpath/128_1024_filter_list.txt

# rm -rf /data/caozhang/program/tmp/existing_db_write 

# ../../build/db_bench --benchmarks="readrandomwriterandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=8 --value_size=1024 --cache_size=134217728 --statistics --use_flash_cache_filter=true --use_local_flash_cache=true --db=/data/caozhang/program/tmp/existing_db_write --flash_cache_size=1024 --num=10000000 --use_flash_cache_filter=true --enable_flash_admission_list=true --enable_flash_evict_blocks=true --enable_flash_prefetch_files=true --dram_cache_size=8 --readwritepercent=90 >$logpath/128_1024_all.txt
