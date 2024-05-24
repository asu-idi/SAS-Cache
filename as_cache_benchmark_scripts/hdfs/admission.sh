logpath="/data/caoz/program/tmp/as_cache_benchmark_results/hdfs/admission/queue"

hdfs dfs -rm -r /tmp/existing_db

../../build/db_bench --benchmarks="readrandomwriterandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=5120 --readwritepercent=70 --cache_size=67108864 --statistics --use_local_flash_cache=false --db=/tmp/existing_db --flash_cache_size=512 --num=5000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 >$logpath/70_64.txt

hdfs dfs -rm -r /tmp/existing_db

../../build/db_bench --benchmarks="readrandomwriterandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=5120 --readwritepercent=70 --cache_size=67108864 --statistics --use_local_flash_cache=true --db=/tmp/existing_db --flash_cache_size=1024 --num=5000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 >$logpath/70_64_1G.txt

hdfs dfs -rm -r /tmp/existing_db

../../build/db_bench --benchmarks="readrandomwriterandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=5120 --readwritepercent=70 --cache_size=67108864 --statistics --use_local_flash_cache=true --db=/tmp/existing_db --flash_cache_size=1024 --num=5000000 --use_flash_cache_filter=false --enable_flash_admission_list=true --dram_cache_size=8 >$logpath/70_64_1G_list.txt

hdfs dfs -rm -r /tmp/existing_db

../../build/db_bench --benchmarks="readrandomwriterandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=5120 --readwritepercent=70 --cache_size=67108864 --statistics --use_local_flash_cache=true --db=/tmp/existing_db --flash_cache_size=1024 --num=5000000 --use_flash_cache_filter=true --enable_flash_admission_list=true --dram_cache_size=8 >$logpath/70_64_1G_list_filter.txt

# hdfs dfs -rm -r /tmp/existing_db

# ../../build/db_bench --benchmarks="readrandomwriterandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=5120 --readwritepercent=90 --cache_size=100663296 --statistics --use_local_flash_cache=false --db=/tmp/existing_db --flash_cache_size=512 --num=5000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 >$logpath/90_96.txt

# hdfs dfs -rm -r /tmp/existing_db

# ../../build/db_bench --benchmarks="readrandomwriterandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=5120 --readwritepercent=90 --cache_size=100663296 --statistics --use_local_flash_cache=true --db=/tmp/existing_db --flash_cache_size=1024 --num=5000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 >$logpath/90_96_1024.txt

# hdfs dfs -rm -r /tmp/existing_db

# ../../build/db_bench --benchmarks="readrandomwriterandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=5120 --readwritepercent=90 --cache_size=100663296 --statistics --use_local_flash_cache=true --db=/tmp/existing_db --flash_cache_size=1024 --num=5000000 --use_flash_cache_filter=false --enable_flash_admission_list=true --dram_cache_size=8 >$logpath/90_96_1024_filter.txt

# ../../build/db_bench --benchmarks="readrandomwriterandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=43 --readwritepercent=50 --cache_size=67108864 --statistics --use_local_flash_cache=false --db=/tmp/existing_db --flash_cache_size=5120 --num=5000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 >$logpath/50_64.txt

# ../../build/db_bench --benchmarks="readrandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=43 --cache_size=67108864 --statistics --use_local_flash_cache=true --db=/tmp/existing_db_write --flash_cache_size=5120 --num=50000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 --read_random_exp_range=5 --reads=5000000 >$logpath/5M_hdd_5_64_5G.txt

# ../../build/db_bench --benchmarks="readrandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=43 --cache_size=67108864 --statistics --use_local_flash_cache=false --db=/tmp/existing_db_write --flash_cache_size=5120 --num=50000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 --read_random_exp_range=5 --reads=10000000 >$logpath/10M_hdd_5_64.txt

# ../../build/db_bench --benchmarks="readrandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=43 --cache_size=67108864 --statistics --use_local_flash_cache=true --db=/tmp/existing_db_write --flash_cache_size=10240 --num=50000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 --read_random_exp_range=5 --reads=10000000 >$logpath/10M_hdd_5_64_10G.txt

# ../../build/db_bench --benchmarks="readrandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=43 --cache_size=67108864 --statistics --use_local_flash_cache=false --db=/tmp/existing_db_write --flash_cache_size=5120 --num=50000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 --read_random_exp_range=5 --reads=20000000 >$logpath/20M_hdd_5_64.txt

# ../../build/db_bench --benchmarks="readrandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=43 --cache_size=67108864 --statistics --use_local_flash_cache=true --db=/tmp/existing_db_write --flash_cache_size=5120 --num=50000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 --read_random_exp_range=5 --reads=20000000 >$logpath/20M_hdd_5_64_5G.txt

# ../../build/db_bench --benchmarks="readrandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=43 --cache_size=67108864 --statistics --use_local_flash_cache=true --db=/tmp/existing_db_write --flash_cache_size=20480 --num=50000000 --use_flash_cache_filter=true --enable_flash_admission_list=false --dram_cache_size=8 --read_random_exp_range=5 --reads=1000000 >$logpath/hdd_5_64_20G_filter.txt

# ../../build/db_bench --benchmarks="readrandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=43 --cache_size=67108864 --statistics --use_local_flash_cache=true --db=/tmp/existing_db_write --flash_cache_size=20480 --num=50000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 --read_random_exp_range=25 --reads=1000000 >$logpath/hdd_25_64_20G.txt

# ../../build/db_bench --benchmarks="readrandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=43 --cache_size=67108864 --statistics --use_local_flash_cache=true --db=/tmp/existing_db_write --flash_cache_size=20480 --num=50000000 --use_flash_cache_filter=true --enable_flash_admission_list=false --dram_cache_size=8 --read_random_exp_range=25 --reads=1000000 >$logpath/hdd_25_64_20G_filter.txt

# ../../build/db_bench --benchmarks="readrandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=43 --cache_size=67108864 --statistics --use_local_flash_cache=true --db=/tmp/existing_db_write --flash_cache_size=20480 --num=50000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 --read_random_exp_range=500 --reads=1000000 >$logpath/hdd_500_64_20G.txt

# ../../build/db_bench --benchmarks="readrandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=43 --cache_size=67108864 --statistics --use_local_flash_cache=true --db=/tmp/existing_db_write --flash_cache_size=20480 --num=50000000 --use_flash_cache_filter=true --enable_flash_admission_list=false --dram_cache_size=8 --read_random_exp_range=500 --reads=1000000 >$logpath/hdd_500_64_20G_filter.txt

# ../../build/db_bench --benchmarks="readrandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=43 --cache_size=268435456 --statistics --use_local_flash_cache=false --db=/tmp/existing_db_write --flash_cache_size=5120 --num=50000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 --read_random_exp_range=25 --reads=1000000 >$logpath/hdd_25_256.txt

# ../../build/db_bench --benchmarks="readrandom,stats" --use_direct_io_for_flush_and_compaction=true --use_direct_reads=true --cache_index_and_filter_blocks=true --perf_level=2 --key_size=48 --value_size=43 --cache_size=268435456 --statistics --use_local_flash_cache=false --db=/tmp/existing_db_write --flash_cache_size=5120 --num=50000000 --use_flash_cache_filter=false --enable_flash_admission_list=false --dram_cache_size=8 --read_random_exp_range=15 --reads=1000000 >$logpath/hdd_15_256.txt

