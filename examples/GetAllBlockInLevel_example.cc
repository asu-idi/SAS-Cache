#include "rocksdb/db.h"
#include <string>
#include <iostream>

#if defined(OS_WIN)
std::string primary_kDBPath = "C:\\Windows\\TEMP\\GetAllBlockInLevel_example_primary_db";
std::string secondary_kDBPath = "C:\\Windows\\TEMP\\GetAllBlockInLevel_example_secondary_db";
#else
std::string primary_kDBPath = "/tmp/GetAllBlockInLevel_example_primary_db";
std::string secondary_kDBPath = "/tmp/GetAllBlockInLevel_example_secondary_db";
#endif

using std::cout;
using std::endl;


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
  }

  std::vector<std::string> block1;
  std::vector<std::string> block2;

  std::vector<std::string> org_block1;
  std::vector<std::string> org_block2;

  int max_keys = 20000;

  std::cout << "Test : read from level 0 " << std::endl;
  {
    uint64_t level = 0;
    s = primary_db->GetAllBlocksInLevel(rocksdb::ReadOptions(), level, block1);

    s = primary_db->GetAllBlocksInLevel(rocksdb::ReadOptions(), level, block2);
    
    assert(block1.empty() == false);
    assert(block2.empty() == false);

    cout << "block1 size: " << block1.size() << " " << "block2 size: " << block2.size() << endl;
    
    assert(block1 == block2);

}
  std::cout << "Test : read from level 0 Successful" << std::endl;

  org_block1 = block1;
  org_block2 = block2;

  block1.clear();
  block2.clear();

  std::cout << "Test : read from level 10(non-exsit) " << std::endl;
  {
    uint64_t level = 10;
    s = primary_db->GetAllBlocksInLevel(rocksdb::ReadOptions(), level, block1);

    s = primary_db->GetAllBlocksInLevel(rocksdb::ReadOptions(), level, block2);
    
    assert(block1.empty() == true);
    assert(block2.empty() == true);

    cout << "block1 size: " << block1.size() << " " << "block2 size: " << block2.size() << endl;
    
    assert(block1 == block2);

}
  std::cout << "Test : read from non-exist level Successful" << std::endl;

}
