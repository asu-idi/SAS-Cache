/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "secondary_cache_cachelib.h"

#include "folly/init/Init.h"
#include "folly/synchronization/Rcu.h"

#include <cstdlib>
#include <ctime>
#include <iostream>

namespace facebook::rocks_secondary_cache {

#define FB_CACHE_MAX_ITEM_SIZE 4 << 20

namespace {
// We use a separate RCU domain since read side critical sections can block
// on IO, and we don't want to interfere with other system activities that
// use RCU synchronization.
folly::rcu_domain& GetRcuDomain() {
  static folly::rcu_domain domain;
  return domain;
}

class RocksCachelibWrapperHandle : public rocksdb::SecondaryCacheResultHandle {
 public:
  RocksCachelibWrapperHandle(folly::SemiFuture<FbCacheReadHandle>&& future,
                             const rocksdb::Cache::CreateCallback& create_cb,
                             std::unique_lock<folly::rcu_domain>&& guard)
      : future_(std::move(future)),
        create_cb_(create_cb),
        val_(nullptr),
        charge_(0),
        is_value_ready_(false),
        guard_(std::move(guard)) {}
  ~RocksCachelibWrapperHandle() override = default;

  RocksCachelibWrapperHandle(const RocksCachelibWrapperHandle&) = delete;
  RocksCachelibWrapperHandle& operator=(const RocksCachelibWrapperHandle&) =
      delete;

  bool IsReady() override {
    bool ready = true;
    if (!is_value_ready_) {
      ready = future_.isReady();
      if (ready) {
        handle_ = std::move(future_).value();
        CalcValue();
      }
    }
    return ready;
  }

  void Wait() override {
    if (!is_value_ready_) {
      future_.wait();
      handle_ = std::move(future_).value();
      CalcValue();
    }
  }

  static void WaitAll(
      std::vector<rocksdb::SecondaryCacheResultHandle*> handles) {
    std::vector<folly::SemiFuture<FbCacheReadHandle>> h_semi;
    for (auto h_ptr : handles) {
      RocksCachelibWrapperHandle* hdl =
          static_cast<RocksCachelibWrapperHandle*>(h_ptr);
      if (hdl->is_value_ready_) {
        continue;
      }
      h_semi.emplace_back(std::move(hdl->future_));
    }
    auto all_handles = folly::collectAll(std::move(h_semi));
    auto new_handles = std::move(all_handles).get();
    assert(new_handles.size() == h_semi.size());
    int result_idx = 0;
    for (size_t i = 0; i < handles.size(); ++i) {
      RocksCachelibWrapperHandle* hdl =
          static_cast<RocksCachelibWrapperHandle*>(handles[i]);
      if (hdl->is_value_ready_) {
        continue;
      }
      hdl->handle_ = std::move(new_handles[result_idx]).value();
      result_idx++;
      hdl->CalcValue();
    }
  }

  void* Value() override { return val_; }

  size_t Size() override { return charge_; }

 private:
  FbCacheReadHandle handle_;
  folly::SemiFuture<FbCacheReadHandle> future_;
  const rocksdb::Cache::CreateCallback& create_cb_;
  void* val_;
  size_t charge_;
  bool is_value_ready_;
  std::unique_lock<folly::rcu_domain> guard_;

  void CalcValue() {
    is_value_ready_ = true;

    if (handle_) {
      uint32_t size = handle_->getSize();
      rocksdb::Status s;

      const char* item = static_cast<const char*>(handle_->getMemory());
      s = create_cb_(item, size, &val_, &charge_);
      if (!s.ok()) {
        val_ = nullptr;
      }
      handle_.reset();
    }
  }
};
} // namespace

RocksCachelibWrapper::~RocksCachelibWrapper() { Close(); }

rocksdb::Status RocksCachelibWrapper::Insert(
    const rocksdb::Slice& key,
    void* value,
    const rocksdb::Cache::CacheItemHelper* helper) {
  rocksdb::Status s;

  if(admissionQueue_ != nullptr) {
    rocksdb::CacheKey rocksdb_block_key;
    memcpy(&rocksdb_block_key, key.data(), key.size());
    uint64_t file_num = rocksdb_block_key.GetFileNum();
    if(admissionQueue_->check(file_num)) {
      std::cout << "reduce_insert_to_as_cache" << std::endl;
      return s;
    }
  }

  std::scoped_lock guard(GetRcuDomain());
  FbCache* cache = cache_.load();

  FbCacheKey k(key.data(), key.size());
  if (cacheFilter_ && !(cacheFilter_->Check(k))) {
    cacheFilter_->Insert(k);
  }
  
  size_t size;

  if (cache) {
    size = (*helper->size_cb)(value);
    if (FbCacheItem::getRequiredSize(k, size) <= FB_CACHE_MAX_ITEM_SIZE) {
      auto handle = cache->allocate(pool_, k, size);
      if (handle) {
        char* buf = static_cast<char*>(handle->getMemory());
        s = (*helper->saveto_cb)(value, /*offset=*/0, size, buf);
        try {
          cache->insertOrReplace(handle);
        } catch (const std::exception& ex) {
          s = rocksdb::Status::Aborted(folly::sformat(
              "Cachelib insertOrReplace exception, error:{}", ex.what()));
        }
      }
      else {
        std::cout << "Cache may fail to evict due to too many pending writes" << std::endl;
      }
    } else {
      s = rocksdb::Status::InvalidArgument();
    }
  }
  return s;
}

rocksdb::Status RocksCachelibWrapper::KeyRangePrefetch(std::string db_name, std::vector<char> smallest_data, std::vector<char> largest_data, int32_t level, std::shared_ptr<rocksdb::SecondaryCache> secondary_cache) {
    rocksdb::Status s;
    rocksdb::DB *secondary_db;
    rocksdb::Options secondary_option;
    rocksdb::BlockBasedTableOptions table_options;
    table_options.use_shared_cache_key = true;
    rocksdb::LRUCacheOptions opts(4 * 1024 /* capacity */, 0 /* num_shard_bits */,
                        false /* strict_capacity_limit */,
                        0.5 /* high_pri_pool_ratio */,
                        nullptr /* memory_allocator */, rocksdb::kDefaultToAdaptiveMutex,
                        rocksdb::kDontChargeCacheMetadata);
    opts.secondary_cache = secondary_cache;
    std::shared_ptr<rocksdb::Cache> cache = rocksdb::NewLRUCache(opts);
    table_options.block_cache = cache;


    secondary_option.table_factory.reset(
                      rocksdb::NewBlockBasedTableFactory(table_options));

    secondary_option.create_if_missing = true;

    srand(time(nullptr));
    int random_number = rand() % (1000 - 1 + 1) + 1;
    std::string secondary_kDBPath = "/tmp/secondary_db/secondary_db";
    s = rocksdb::DB::OpenAsSecondary(secondary_option, db_name, secondary_kDBPath + std::to_string(random_number), &secondary_db);
    assert(s.ok());

    secondary_db->TryCatchUpWithPrimary();

    std::vector<rocksdb::Slice> content;
    rocksdb::Slice min_keys(smallest_data.data(), smallest_data.size());
    rocksdb::Slice max_keys(largest_data.data(), largest_data.size());
    // TODO: remove content. Here content is none, we choose to directly insert the blocks into the secondary cache when reading it from file and don't collect it all out and insert.
    secondary_db->GetBlockInKeyRange(rocksdb::ReadOptions(), level, min_keys, max_keys, content);
    secondary_db->Close();

    return rocksdb::Status::OK();
}

rocksdb::Status RocksCachelibWrapper::RecordInvalidFiles(const std::string& db_name, const std::vector<uint64_t>& files) {
    (void)db_name;
    if (admissionQueue_ == nullptr) {
      return rocksdb::Status::OK();
    }
    for(auto it = files.begin(); it != files.end(); ++it) {
      admissionQueue_->insert(*it);
    }
    return rocksdb::Status::OK();
}

rocksdb::Status RocksCachelibWrapper::EvictInvalidBlocks(const std::vector<rocksdb::CacheKey>& blocks) {
    for(auto it = blocks.begin(); it != blocks.end(); ++it) {
        rocksdb::Slice slice_key = (*it).AsSlice();
        Erase(slice_key);
    }
    return rocksdb::Status::OK();
}

std::unique_ptr<rocksdb::SecondaryCacheResultHandle>
RocksCachelibWrapper::Lookup(const rocksdb::Slice& key,
                             const rocksdb::Cache::CreateCallback& create_cb,
                             bool wait,
                             bool /*advise_erase*/,
                             bool& is_in_sec_cache) {
  std::unique_lock guard(GetRcuDomain());
  FbCache* cache = cache_.load();

  if (cacheFilter_ && !(cacheFilter_->Check(FbCacheKey(key.data(), key.size())))) {
    is_in_sec_cache = false;
    return nullptr;
  }

  std::unique_ptr<rocksdb::SecondaryCacheResultHandle> hdl;

  if (cache) {
    auto handle = cache->find(FbCacheKey(key.data(), key.size()));
    // We cannot dereference the handle in anyway. Any dereference will make it
    // synchronous, so get the SamiFuture right away
    // std::move the std::unique_lock<rcu_domain> (reader lock) to the
    // RocksCachelibWrapperHandle, and will be released when the handle is
    // destroyed.
    hdl = std::make_unique<RocksCachelibWrapperHandle>(
        std::move(handle).toSemiFuture(),
        create_cb,
        std::move(guard));
    if (hdl->IsReady() || wait) {
      if (!hdl->IsReady()) { // WART: double-call IsReady()
        hdl->Wait();
      }
      if (hdl->Value() == nullptr) {
        hdl.reset();
      }
    }
  }

  is_in_sec_cache = hdl != nullptr;
  return hdl;
}

void RocksCachelibWrapper::Erase(const rocksdb::Slice& key) {
  std::scoped_lock guard(GetRcuDomain());
  FbCache* cache = cache_.load();


  if (cacheFilter_) {
    cacheFilter_->Remove(FbCacheKey(key.data(), key.size()));
  }
  if (cache) {
    cache->remove(FbCacheKey(key.data(), key.size()));
  }
}

void RocksCachelibWrapper::WaitAll(
    std::vector<rocksdb::SecondaryCacheResultHandle*> handles) {
  RocksCachelibWrapperHandle::WaitAll(handles);
}

void RocksCachelibWrapper::Close() {
  FbCache* cache = cache_.load();
  if (cache) {
    // Nullify the cache pointer, then wait for all read side critical
    // sections already started to finish, and then delete the cache
    cache_.store(nullptr);
    GetRcuDomain().synchronize();
    delete cache;
  }
}

// Global cache object and a default cache pool
std::shared_ptr<rocksdb::SecondaryCache> NewRocksCachelibWrapper(unsigned long flash_cache_size, bool enable_cache_filter, bool enable_admission_queue, bool enable_replacement) {

  std::string NavyFileNameBase = "/nvme/cachelib/NavyStorage";
  srand(time(nullptr));
  int random_number = rand() % (1000 - 1 + 1) + 1;
  std::string NavyFileName = NavyFileNameBase + std::to_string(random_number);

  RocksCachelibOptions opts;
  opts.cacheName = "SecondaryCacheCachelib";
  opts.fileName = NavyFileName;
  opts.size = flash_cache_size * 1024UL * 1024UL;

  std::unique_ptr<FbCache> cache;
  cachelib::PoolId defaultPool;
  FbCacheConfig config;
  NvmCacheConfig nvmConfig;

  nvmConfig.navyConfig.setBlockSize(opts.blockSize);
  nvmConfig.navyConfig.setSimpleFile(opts.fileName,
                                     opts.size,
                                     /*truncateFile=*/false);
  nvmConfig.navyConfig.blockCache().setRegionSize(opts.regionSize);

  config.setCacheSize(opts.volatileSize)
      .setCacheName(opts.cacheName)
      .setAccessConfig(
          {opts.bktPower /* bucket power */, opts.lockPower /* lock power */})
      .enableNvmCache(nvmConfig)
      .validate(); // will throw if bad config

  return std::shared_ptr<rocksdb::SecondaryCache>(new RocksCachelibWrapper(config, flash_cache_size, enable_cache_filter, enable_admission_queue, enable_replacement));
}

} // namespace facebook::rocks_secondary_cache