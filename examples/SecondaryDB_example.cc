#include "rocksdb/db.h"
#include "rocksdb/table.h"
#include "rocksdb/filter_policy.h"
#include <string>
#include <iostream>

#if defined(OS_WIN)
std::string primary_kDBPath = "C:\\Windows\\TEMP\\SecondaryDB_example_primary_db";
std::string secondary_kDBPath = "C:\\Windows\\TEMP\\SecondaryDB_example_secondary_db";
#else
std::string primary_kDBPath = "/tmp/SecondaryDB_example_primary_db";
std::string secondary_kDBPath = "/tmp/SecondaryDB_example_secondary_db";
#endif

using std::cout;
using std::endl;
using ROCKSDB_NAMESPACE::Slice;



int main(int argc, const char *argv[]) {
  bool new_set_up = false;
  if(argv[1]) {
    new_set_up = true;
  }

  int max_number = 0;
  int target_number = 0;
  rocksdb::Status s;

  rocksdb::DB *primary_db;
  rocksdb::Options option;
  rocksdb::DB *secondary_db;
  rocksdb::Options secondary_option;

  rocksdb::BlockBasedTableOptions table_options;
  table_options.filter_policy.reset(rocksdb::NewBloomFilterPolicy(10, false));
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

  std::vector<Slice> block1;
  std::vector<Slice> block2;

  int max_keys = 20000;

  std::cout << "GetAllBlocksInLevel Test : read from level 0 " << std::endl;
  {
    uint64_t level = 0;
    s = primary_db->GetAllBlocksInLevel(rocksdb::ReadOptions(), level, block1);

    s = secondary_db->GetAllBlocksInLevel(rocksdb::ReadOptions(), level, block2);
    
    assert(block1.empty() == false);
    assert(block2.empty() == false);

    // cout << "block1 size: " << block1.size() << " " << "block2 size: " << block2.size() << endl;
    
    assert(block1 == block2);

}
  std::cout << "GetAllBlocksInLevel Test : read from level 0 Successful" << std::endl;

  block1.clear();
  block2.clear();

  std::cout << "GetBlockInLevel Test : read from the same key " << std::endl;
  {
    target_number = 10;
    std::string key1 = "hello" + std::to_string(target_number);

    uint64_t level = 0;
    auto read_option = rocksdb::ReadOptions();
    read_option.get_del_range_block = true;
    read_option.get_filter_block = true;
    read_option.get_index_block = true;
    s = primary_db->GetBlockInLevel(read_option, key1, level, block1);

    s = secondary_db->GetBlockInLevel(read_option, key1, level, block2);

    cout << "block1 size: " << block1.size() << " " << "block2 size: " << block2.size() << endl;
    // cout << "block1 layout: " << block1[block1.size()-1] << " " << "block2 layout: " << block2[block2.size() - 1] << endl;

    assert(block1.empty() == false);
    assert(block2.empty() == false);
    assert(block1 == block2);

}
  std::cout << "GetBlockInLevel Test : read from the same key Successful" << std::endl;


  std::cout << "GetBlockInKeyRange Test : read from the same key " << std::endl;
  {
    std::vector<Slice> key1;
    for(int i = 0;i < max_keys; ++i) {
        std::string key = "hello" + std::to_string(i);
        key1.push_back(Slice(key));
    }

    uint64_t level = 0;
    s = primary_db->GetBlockInKeyRange(rocksdb::ReadOptions(), level, key1[0], key1[key1.size()-1], block1);

    s = secondary_db->GetBlockInKeyRange(rocksdb::ReadOptions(), level, key1[0], key1[key1.size()-1], block2);

    cout << "block1 size: " << block1.size() << " " << "block2 size: " << block2.size() << endl;
    assert(block1.empty() == false);
    assert(block2.empty() == false);

    assert(block1 == block2);

}
  std::cout << "GetBlockInKeyRange Test : read from the same key Successful" << std::endl;

  primary_db->Close();
  secondary_db->Close();
}
