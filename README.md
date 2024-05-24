# SAS-Cache: A Semantic-Aware Secondary Cache for LSM-based Key-Value Stores

## Setup

### Setup Cachelib

Following the instructions in the cachelib websites: https://cachelib.org/docs/installation

For example: 

`git clone https://github.com/facebook/CacheLib`

`cd CacheLib`

`./contrib/build.sh -j -T`

### Setup RocksDB with Cachelib

#### Modify the path of Cachelib in the `build.sh`

change the following line to your own cachelib path: 

`CLBASE="/data/gc/programs/CacheLib"`

#### Build RocksDB with Cachelib

##### Get the googletest:

`git clone git@github.com:google/googletest.git third-party/googletest`

##### Set the FindModule sybolic link

`sudo ln -s /(path-to-cachelib)/CacheLib/cachelib/cmake/FindSodium.cmake /usr/local/share/cmake-3.23/Modules`

**You may need to update your own cmake modules path.**

##### Run the following command: 

`bash build.sh`

### Run RocksDB with Cachelib

After above configrations, you can run RocksDB with Cachelib-based Secondary Cache.

For example, you can run the following command: 

`bash run.sh`

here you may need to run with sudo `sudo bash run.sh`

#### Flags in db_bench related to SAS-Cache

`--use_local_flash_cache` : default is false

`--flash_cache_size`(MB): default is 5120 (5GB)

`--use_flash_cache_filter`: default is false

`--enable_flash_admission_list`: default is false

`--enable_flash_evict_blocks`: default is false

`--enable_flash_prefetch_files`: default is false

### Misc

#### Run with HDFS

If you want to run with HDFS, you can directly change the following line in CMakeList.txt, change `OFF` to `ON`. 

`option(WITH_HDFS "build with HDFS and write data to HDFS" OFF)`
