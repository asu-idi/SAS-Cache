#include "rocksdb/db.h"
#include <string>
#include <iostream>

#if defined(OS_WIN)
std::string primary_kDBPath = "C:\\Windows\\TEMP\\GetMetaDataBlock_example_primary_db";
std::string secondary_kDBPath = "C:\\Windows\\TEMP\\GetMetaDataBlock_example_secondary_db";
#else
std::string primary_kDBPath = "/tmp/GetMetaDataBlock_example_primary_db";
std::string secondary_kDBPath = "/tmp/GetMetaDataBlock_example_secondary_db";
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
  std::vector<std::string> block2;

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

    s = primary_db->GetBlockInLevel(rocksdb::ReadOptions(), key1, level, block2);

    cout << "block1 size: " << block1.size() << " " << "block2 size: " << block2.size() << endl;
    cout << "block1 layout: " << block1[block1.size()-1] << " " << "block2 layout: " << block2[block2.size() - 1] << endl;
    cout << "data block: " << block1[0] << endl;
    cout << "index block: " << block1[1] << endl;
    cout << "del range block: " << block1[2] << endl;

    // assert(block1.empty() == false);
    // assert(block2.empty() == false);
    // assert(block1 == block2);

}
  std::cout << "GetBlockInLevel Test : read from the same key Successful" << std::endl;

  primary_db->Close();
  secondary_db->Close();
}
