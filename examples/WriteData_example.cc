
#include "rocksdb/cache.h"
#include "rocksdb/db.h"
#include "rocksdb/table.h"
#include "rocksdb/filter_policy.h"

#include <string>
#include <iostream>

#if defined(OS_WIN)
std::string primary_kDBPath = "C:\\Windows\\TEMP\\GetBlockAndCacheKeyPlus_primary_db";
#else
std::string primary_kDBPath = "/tmp/GetBlockAndCacheKeyPlus_primary_db";
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
  rocksdb::BlockBasedTableOptions table_options;
  table_options.filter_policy.reset(rocksdb::NewBloomFilterPolicy(10, false));
  option.table_factory.reset(
      rocksdb::NewBlockBasedTableFactory(table_options));
  option.create_if_missing = true;

  s = rocksdb::DB::Open(option, primary_kDBPath, &primary_db);
  assert(s.ok());
  
  max_number = 2000000;

    // just put data 
    rocksdb::FlushOptions flush_option;
    for (int i = 0; i < max_number; i++) {
      s = primary_db->Put(rocksdb::WriteOptions(), "hello" + std::to_string(i),
                          "hello");

      assert(s.ok());
    }
    s = primary_db->Flush(flush_option);
    assert(s.ok());

    // Delete part of the data
    for (int i = 0; i < max_number / 10; i++) {
      s = primary_db->Delete(rocksdb::WriteOptions(), "hello" + std::to_string(i));
      assert(s.ok());
    }
    s = primary_db->Flush(flush_option);
    assert(s.ok());

    // rewrite the deleted data
    for (int i = 0; i < max_number / 10; i++) {
      s = primary_db->Put(rocksdb::WriteOptions(), "hello" + std::to_string(i),
                          "hello" + std::to_string(i));
      assert(s.ok());
    }
    s = primary_db->Flush(flush_option);
    assert(s.ok());

  primary_db->Close();
}
