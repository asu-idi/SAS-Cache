// This file is used to test the cache key. we make the cache key shared by the primary db and its secondary dbs.
// we do the following tests
// 1. use a primary db , first use get block and cache it into block cache,
//  then use get function, the target key is contained in the block which is cached in block cache
//  it's obviously that there will no need to fetch the block from file, we can see this
// 2. In this case, we use the default cache key which the primary db and secondary db don't share. 
//  First use a secondary db , use get block and cache it into block cache,
//  then use the primary db to get target key, the target key is contained in the block which is cached in block cache
//  But in this case, the primaty db will not see the cached block, so it has to get from file, we can see this.
// 3. In this case, we use the modified cache key which the primary db and secondary db share. 
//  First use a secondary db , use get block and cache it into block cache,
//  then use the primary db to get target key, the target key is contained in the block which is cached in block cache
//  In this case, the primaty db will see the cached block(because it share the cache with secondary db), 
//  so it doesn't have to get from file, we can see this.

// If you want to see the result, you need to modify a line in block_based_table_reader.cc::MaybeReadBlockAndLoadToCache,
// about 1684 line. it prints "enter get from file"

// Usage : just run `./GetBlockAndCacheKey_example.cc 1`

#include "rocksdb/cache.h"
#include "rocksdb/db.h"
#include "rocksdb/table.h"

#include <string>
#include <iostream>

#if defined(OS_WIN)
std::string primary_kDBPath = "C:\\Windows\\TEMP\\GetBlockAndCacheKey_primary_db";
std::string secondary_kDBPath = "C:\\Windows\\TEMP\\GetBlockAndCacheKey_secondary_db";
#else
std::string primary_kDBPath = "/tmp/GetBlockAndCacheKey_primary_db";
std::string secondary_kDBPath = "/tmp/GetBlockAndCacheKey_secondary_db";
#endif

using std::cout;
using std::endl;
using ROCKSDB_NAMESPACE::Slice;
using ROCKSDB_NAMESPACE::NewLRUCache;



int main(int argc, const char *argv[]) {
  bool new_set_up = false;
  if(argv[1]) {
    new_set_up = true;
  }

  int max_number = 0;
  int target_number = 0;
  int delta_number = 0;
  rocksdb::Status s;

  rocksdb::DB *primary_db;
  rocksdb::Options option;
  rocksdb::DB *secondary_db;
  rocksdb::Options secondary_option;
  rocksdb::BlockBasedTableOptions table_options;
  table_options.block_cache = NewLRUCache(4096 * 1000 * 100);
  table_options.use_shared_cache_key = true;

  option.table_factory.reset(
      rocksdb::NewBlockBasedTableFactory(table_options));

  secondary_option.table_factory.reset(
      rocksdb::NewBlockBasedTableFactory(table_options));


  option.create_if_missing = true;

  secondary_option.create_if_missing = true;

  s = rocksdb::DB::Open(option, primary_kDBPath, &primary_db);
  assert(s.ok());

  std::string db_name = primary_db->GetName();
  s = rocksdb::DB::OpenAsSecondary(secondary_option, db_name, secondary_kDBPath, &secondary_db);
  assert(s.ok());
  
  max_number = 2000000;

  if(new_set_up) {
    // set primary_db
    for (int i = 0; i < max_number; i++) {
      s = primary_db->Put(rocksdb::WriteOptions(), "hello" + std::to_string(i),
                          "hello");

      assert(s.ok());
    }
    rocksdb::FlushOptions flush_option;
    s = primary_db->Flush(flush_option);
    assert(s.ok());
  }

  secondary_db->TryCatchUpWithPrimary();  // This is crutial

  std::vector<std::string> block1;

  std::cout << "GetBlockInLevel Test" << std::endl;
  {
    target_number = 20;

    uint64_t level = 0;
    auto read_option = rocksdb::ReadOptions();

    std::cout << "GetBlock begin" << std::endl;
    s = secondary_db->GetBlockInLevel(read_option, Slice("hello" + std::to_string(target_number)), level, block1);
    std::cout << "GetBlock end" << std::endl;

    std::string value;
    std::cout << "Get begin" << std::endl;
    s = primary_db->Get(rocksdb::ReadOptions(), Slice("hello" + std::to_string(target_number)), &value);
    std::cout << "Get end" << std::endl;

}
  std::cout << "GetBlockInLevel Test" << std::endl;

// you can test this as well, I comment this because it may disturb the correctness of the GetBlockInKeyRange
// So you would better test them seperately.

    // block1.clear();

//   std::cout << "GetAllBlocksInLevel Test" << std::endl;
//   {
//     uint64_t level = 0;
//     auto read_option = rocksdb::ReadOptions();

//     std::cout << "GetBlock begin" << std::endl;
//     s = secondary_db->GetAllBlocksInLevel(read_option, level, block1);
//     std::cout << "GetBlock end" << std::endl;

//     std::string value;
//     std::cout << "Get begin" << std::endl;
//     for(auto i = 0; i < max_number; ++i) {
//       // std::cout << "Key :" << "hello" + std::to_string(i) << endl;
//       s = primary_db->Get(rocksdb::ReadOptions(), Slice("hello" + std::to_string(i)), &value);
//     }
//     std::cout << "Get end" << std::endl;

// }
//   std::cout << "GetAllBlocksInLevel Test" << std::endl;

  block1.clear();

  std::cout << "GetBlockInLevelRange Test" << std::endl;
  {
    target_number = 0;
    delta_number = 1000000 - 1;
    std::string key1 = "hello" + std::to_string(target_number);
    std::string key2 = "hello" + std::to_string(target_number + delta_number);
    std::string key3 = "hello" + std::to_string(target_number + delta_number + 1000000);
    std::string key4 = "hello" + std::to_string(1365038);

    uint64_t level = 0;
    auto read_option = rocksdb::ReadOptions();
    std::cout << "GetBlock begin" << std::endl;
    s = secondary_db->GetBlockInKeyRange(read_option, level, key1, key2, block1);
    s = secondary_db->GetBlockInKeyRange(read_option, level, key4, key3, block1);
    std::cout << "GetBlock end" << std::endl;

    std::string value;
    std::cout << "Get begin" << std::endl;
    for(auto i = target_number; i < target_number + delta_number + 1; ++i) {
      // std::cout << "Key :" << "hello" + std::to_string(i) << endl;
      s = primary_db->Get(rocksdb::ReadOptions(), Slice("hello" + std::to_string(i)), &value);
    }
    std::cout << "Get end" << std::endl;

}
  std::cout << "GetBlockInLevelRange Test" << std::endl;

  primary_db->Close();
  secondary_db->Close();
}
