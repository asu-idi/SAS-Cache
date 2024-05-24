//
// Created by Tom cao on 2022/9/14.
//

#include "rocksdb/db.h"
#include <string>
#include <iostream>

#if defined(OS_WIN)
std::string primary_kDBPath = "C:\\Windows\\TEMP\\GetBlockInLevelRange_example_primary_db";
std::string secondary_kDBPath = "C:\\Windows\\TEMP\\GetBlockInLevelRange_example_secondary_db";
#else
std::string primary_kDBPath = "/tmp/GetBlockInLevelRange_example_primary_db";
std::string secondary_kDBPath = "/tmp/GetBlockInLevelRange_example_secondary_db";
#endif

using ROCKSDB_NAMESPACE::Slice;

int main(int argc, const char *argv[]) {
  bool new_set_up = false;
  if(argv[1]) {
    new_set_up = true;
  }

  int max_number = 0;
  int target_number = 0;
  rocksdb::Status s;
  rocksdb::DB *secondary_db;
  rocksdb::Options secondary_option;

  secondary_option.create_if_missing = true;

  s = rocksdb::DB::Open(secondary_option, secondary_kDBPath, &secondary_db);
  assert(s.ok());

  rocksdb::DB *primary_db;
  rocksdb::Options option;

  option.create_if_missing = true;

  s = rocksdb::DB::Open(option, primary_kDBPath, &primary_db);
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


    // set secondary_db
    for (int i = 0; i < max_number; i++) {
      s = secondary_db->Put(rocksdb::WriteOptions(), "hello" + std::to_string(i),
                            "hello");

      assert(s.ok());
    }
    s = secondary_db->Flush(flush_option);
    assert(s.ok());
  }


  std::vector<std::string> block1;
  std::vector<std::string> block2;

  std::vector<std::string> org_block1;
  std::vector<std::string> org_block2;


  std::cout << "Test : read from the same key " << std::endl;
  {
    target_number = 10;
    std::string key1 = "hello" + std::to_string(target_number);

    uint64_t level = 0;
    s = primary_db->GetBlockInLevelRange(rocksdb::ReadOptions(), key1, level, level + 5, block1);

    s = secondary_db->GetBlockInLevelRange(rocksdb::ReadOptions(), key1, level, level+ 5, block2);

    assert(block1.empty() == false);
    assert(block2.empty() == false);
    assert(block1 == block2);

}
  std::cout << "Test : read from the same key Successful" << std::endl;

  org_block1 = block1;
  org_block2 = block2;

  block1.clear();
  block2.clear();

  std::cout << "Test : read from non-exist key " << std::endl;
  {
    target_number = 10;
    std::string key1 = "hello" + std::to_string(target_number);

    uint64_t level = 8;
    s = primary_db->GetBlockInLevelRange(rocksdb::ReadOptions(), key1, level,level + 5, block1);

    s = secondary_db->GetBlockInLevelRange(rocksdb::ReadOptions(), key1, level,level+5 , block2);

    assert(block1.empty() == true);
    assert(block2.empty() == true);
    assert(block1 == block2);

}
  std::cout << "Test : read from non-exist key Successful" << std::endl;


    // deleta keys
  if(new_set_up) {
    // delete keys in primary_db
    for (int i = 0; i < max_number/10; i++) {
      s = primary_db->Delete(rocksdb::WriteOptions(), "hello" + std::to_string(i));

      assert(s.ok());
    }
    rocksdb::FlushOptions flush_option;
    s = primary_db->Flush(flush_option);
    assert(s.ok());


    // delete keys in secondary_db
    for (int i = 0; i < max_number/10; i++) {
      s = secondary_db->Delete(rocksdb::WriteOptions(), "hello" + std::to_string(i));

      assert(s.ok());
    }
    s = secondary_db->Flush(flush_option);
    assert(s.ok());
  }

  std::cout << "Test : read from the existing key after deletion" << std::endl;
  {
    target_number = 100000;
    std::string key1 = "hello" + std::to_string(target_number);

    uint64_t level = 0;
    s = primary_db->GetBlockInLevelRange(rocksdb::ReadOptions(), key1, level,level+5, block1);

    s = secondary_db->GetBlockInLevelRange(rocksdb::ReadOptions(), key1, level,level+5, block2);

    assert(block1.empty() == false);
    assert(block2.empty() == false);
    assert(block1 == block2);

}
  std::cout << "Test : read from the existing key after deletion Successful" << std::endl;

  block1.clear();
  block2.clear();

    // add new keys
  if(new_set_up) {
    // add new keys in primary_db
    for (int i = 0; i < max_number/10; i++) {
      s = primary_db->Put(rocksdb::WriteOptions(), "hello" + std::to_string(i),
                          "hello" + std::to_string(i));

      assert(s.ok());
    }
    rocksdb::FlushOptions flush_option;
    s = primary_db->Flush(flush_option);
    assert(s.ok());


    // add new keys in secondary_db
    for (int i = 0; i < max_number/10; i++) {
      s = secondary_db->Put(rocksdb::WriteOptions(), "hello" + std::to_string(i),
                          "hello" + std::to_string(i));

      assert(s.ok());
    }
    s = secondary_db->Flush(flush_option);
    assert(s.ok());
  }


  std::cout << "Test : read from the same key after adding" << std::endl;
  {
    target_number = 10;
    std::string key1 = "hello" + std::to_string(target_number);

    uint64_t level = 0;
    s = primary_db->GetBlockInLevelRange(rocksdb::ReadOptions(), key1, level,level+5,  block1);

    s = secondary_db->GetBlockInLevelRange(rocksdb::ReadOptions(), key1, level, level+5, block2);

    assert(block1.empty() == false);
    assert(block2.empty() == false);
    assert(block1 == block2);

}
  std::cout << "Test : read from the same key after adding Successful" << std::endl;

  std::cout << "Test : the origin and last block is different" << std::endl;

    assert(block1 != org_block1);
    assert(block2 != org_block2);

  std::cout << "Test : the origin and last block is different Successful" << std::endl;

}
