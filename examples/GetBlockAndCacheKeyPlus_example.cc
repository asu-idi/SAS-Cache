//Usage : first run the WriteData_example, then run this file


#include "rocksdb/cache.h"
#include "rocksdb/db.h"
#include "rocksdb/table.h"
#include "rocksdb/filter_policy.h"

#include <string>
#include <iostream>

#if defined(OS_WIN)
std::string primary_kDBPath = "C:\\Windows\\TEMP\\GetBlockAndCacheKeyPlus_primary_db";
std::string secondary_kDBPath = "C:\\Windows\\TEMP\\GetBlockAndCacheKeyPlus_secondary_db";
#else
std::string primary_kDBPath = "/tmp/GetBlockAndCacheKeyPlus_primary_db";
std::string secondary_kDBPath = "/tmp/GetBlockAndCacheKeyPlus_secondary_db";
#endif

using std::cout;
using std::endl;
using ROCKSDB_NAMESPACE::Slice;
using ROCKSDB_NAMESPACE::NewLRUCache;



int main(int argc, const char *argv[]) {
  int max_number = 0;
  int target_number = 0;
  int delta_number = 0;
  rocksdb::Status s;

  rocksdb::DB *primary_db;
  rocksdb::Options option;
  rocksdb::DB *secondary_db;
  rocksdb::Options secondary_option;
  rocksdb::BlockBasedTableOptions table_options;
  table_options.filter_policy.reset(rocksdb::NewBloomFilterPolicy(10, false));
  table_options.block_cache = NewLRUCache(4096 * 1000 * 100);
  table_options.use_shared_cache_key = true;
  // table_options.cache_index_and_filter_blocks = true;

  secondary_option.table_factory.reset(
      rocksdb::NewBlockBasedTableFactory(table_options));


  option.create_if_missing = true;

  secondary_option.create_if_missing = true;

  std::string db_name = primary_kDBPath;
  s = rocksdb::DB::OpenAsSecondary(secondary_option, db_name, secondary_kDBPath, &secondary_db);
  assert(s.ok());
  
  max_number = 2000000;

  secondary_db->TryCatchUpWithPrimary();  // This is crutial

  std::vector<Slice> block1;

  std::cout << "GetBlockInLevel Test" << std::endl;
  {
    target_number = 20;

    uint64_t level = 0;
    auto read_option = rocksdb::ReadOptions();

    std::cout << "GetBlock begin" << std::endl;
    read_option.get_index_block = true;
    // read_option.get_filter_block = true;
    s = secondary_db->GetBlockInLevel(read_option, Slice("hello" + std::to_string(target_number)), level, block1);
    cout << "block1 layout: " << block1[block1.size()-1] << endl;
    std::cout << "GetBlock end" << std::endl;

    std::string value;

    table_options.cache_index_and_filter_blocks = true;
    option.table_factory.reset(
        rocksdb::NewBlockBasedTableFactory(table_options));
    s = rocksdb::DB::Open(option, primary_kDBPath, &primary_db);
    assert(s.ok());
    std::cout << "Get begin" << std::endl;

    s = primary_db->Get(rocksdb::ReadOptions(), Slice("hello" + std::to_string(target_number)), &value);
    std::cout << "Get end" << std::endl;

}
  std::cout << "GetBlockInLevel Test finish" << std::endl;

// you can test this as well, I comment this because it may disturb the correctness of the GetBlockInKeyRange
// So you would better test them seperately.

//   block1.clear();

//   std::cout << "GetAllBlocksInLevel Test" << std::endl;
//   {
//     uint64_t level = 1;
//     auto read_option = rocksdb::ReadOptions();

//     std::cout << "GetBlock begin" << std::endl;
//     s = secondary_db->GetAllBlocksInLevel(read_option, level, block1);
//     std::cout << "GetBlock end" << std::endl;

//     std::string value;
//     std::cout << "Get begin" << std::endl;
//     for(auto i = 0; i < max_number; ++i) {
//       // std::cout << "Key :" << "hello" + std::to_string(i) << endl;
//       s = primary_db->Get(rocksdb::ReadOptions(), Slice("hello" + std::to_string(i)), &value);
//       if(i >= 0 && i < max_number/10) {
//         assert(value == "hello" + std::to_string(i));
//       }
//       else {
//         assert(value == "hello");
//       }
//     }
//     std::cout << "Get end" << std::endl;

// }
//   std::cout << "GetAllBlocksInLevel Test" << std::endl;

  // block1.clear();

//   std::cout << "GetBlockInLevelRange Test" << std::endl;
//   {
//     target_number = 0;
//     delta_number = 1000000 - 1;
//     std::string key1 = "hello" + std::to_string(target_number);
//     std::string key2 = "hello" + std::to_string(target_number + delta_number);
//     std::string key3 = "hello" + std::to_string(target_number + delta_number + 1000000);
//     std::string key4 = "hello" + std::to_string(1365038);

//     uint64_t level = 1;
//     auto read_option = rocksdb::ReadOptions();
//     std::cout << "GetBlock begin" << std::endl;
//     s = secondary_db->GetBlockInKeyRange(read_option, level, key1, key2, block1);
//     s = secondary_db->GetBlockInKeyRange(read_option, level, key4, key3, block1);
//     std::cout << "GetBlock end" << std::endl;

//     std::string value;
//     std::cout << "Get begin" << std::endl;
//     for(auto i = target_number; i < target_number + delta_number + 1; ++i) {
//       s = primary_db->Get(rocksdb::ReadOptions(), Slice("hello" + std::to_string(i)), &value);
//       // check the readed value.
//       if(i >= 0 && i < max_number/10) {
//         assert(value == "hello" + std::to_string(i));
//       }
//       else {
//         assert(value == "hello");
//       }
//     }
//     std::cout << "Get end" << std::endl;

// }
//   std::cout << "GetBlockInLevelRange Test" << std::endl;

  primary_db->Close();
  secondary_db->Close();
}
