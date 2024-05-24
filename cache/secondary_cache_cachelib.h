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

#pragma once
#include "cachelib/allocator/CacheAllocator.h"
#include "rocksdb/secondary_cache.h"
#include "rocksdb/db.h"
#include "rocksdb/table.h"
#include "rocksdb/types.h"
#include "rocksdb/cache.h"
#include "cache_filter/cuckoofilter.h"
#include "cache/cache_key.h"


using cuckoofilter::CuckooFilter;
using cuckoofilter::HashUtil;

namespace facebook {
namespace rocks_secondary_cache {


using FbCache =
    cachelib::LruAllocator; // or Lru2QAllocator, or TinyLFUAllocator
using FbCacheConfig = typename FbCache::Config;
using NvmCacheConfig = typename FbCache::NvmCacheConfig;
using FbCacheKey = typename FbCache::Key;
using FbCacheReadHandle = typename FbCache::ReadHandle;
using FbCacheItem = typename FbCache::Item;
using DestructorData = typename FbCache::DestructorData;
using DestructorContext = facebook::cachelib::DestructorContext;

// cache filter implementation
class CacheFilter {
public:
    CacheFilter(uint64_t total_items) {
        filter = new CuckooFilter<uint32_t, 32>(total_items);
    }

    void Insert(const FbCacheKey& cache_key) {
        std::string key = CacheKeyToStr(cache_key);
        if (filter->Add(HashUtil::SuperFastHash(key)) != cuckoofilter::Ok) {
            std::cout << "Insert_failed, key: " << key << std::endl;
        }
    }

    bool Check(const FbCacheKey& cache_key) {
        std::string key = CacheKeyToStr(cache_key);
        auto result = filter->Contain(HashUtil::SuperFastHash(key));
        if (result != cuckoofilter::Ok) {
            // std::cout << "Check failed, key: " << key << std::endl;
            return false;
        }
        return true;
    }

    void Remove(const FbCacheKey& cache_key) {
        std::string key = CacheKeyToStr(cache_key);
        auto start_time = std::chrono::high_resolution_clock::now();
        if (filter->Delete(HashUtil::SuperFastHash(key)) != cuckoofilter::Ok) {
            std::cout << "Remove_failed, key: " << key << std::endl;
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
        // std::cout << "cache_filter_remove Time: " << duration << "us" << std::endl;
    }

private:
    std::string CacheKeyToStr(const FbCacheKey& cache_key) {
        return std::string(cache_key.data(), cache_key.size());
    }

    CuckooFilter<uint32_t, 32>* filter;
};


// admission queue implementation
class AdmissionQueue {
public:
    AdmissionQueue(uint64_t capacity) {
        maxSize = capacity;
    }

    bool check(uint64_t key) {
        std::shared_lock<std::shared_mutex> lock(mutex);
        auto setIt = keySet.find(key);
        if (setIt != keySet.end()) {
            return true;
        }
        return false;
    }

    void insert(uint64_t key) {
        std::unique_lock<std::shared_mutex> lock(mutex);
        if (keySet.find(key) != keySet.end()) {
            return;
        }
        if (keyQueue.size() == maxSize) {
            keySet.erase(keyQueue.front());
            keyQueue.pop();
        }
        keySet.insert(key);
        keyQueue.push(key);
    }

private:
    uint64_t maxSize;
    std::queue<uint64_t> keyQueue;
    std::set<uint64_t> keySet;
    std::shared_mutex mutex;
};

// Options structure for configuring a Cachelib SecondaryCache instance
struct RocksCachelibOptions {
  // A name for the use case
  std::string cacheName;

  // Path to the cache file
  std::string fileName;

  // Maximum size of the cache file
  unsigned long size;

  // Minimum IO granularity. Typically the device block size
  size_t blockSize = 4096;

  // Size of a cache region. A region is the granularity for garbage
  // collection
  size_t regionSize = 16 * 1024 * 1024;

  // Admission control policy - random or dynamic_random. The latter allows
  // writes to be rate limited in order to prolong flash life, and the
  // rejection rate is dynamically adjusted to ratelimit writes.
  std::string admPolicy = "random";

  // For random admission policy, probability of admission
  double admProbability = 1.0;

  // Maximum write rate for dynamic_random policy in bytes/s
  uint64_t maxWriteRate = 128 << 20;

  // Target daily write rate for dynamic_random policy in bytes/s. This would
  // typically be <= maxWriteRate.
  uint64_t admissionWriteRate = 128 << 20;

  // Size of the volatile portion of the cache. Typically a few 10s to a
  // couple of 100s of MBs
  size_t volatileSize = 32 * 1024 * 1024;

  // Base 2 exponent for number of hash table buckets
  uint32_t bktPower = 12;

  // Base 2 exponent for number of locks
  uint32_t lockPower = 12;

  // An oncall name for FB303 stats
  std::string oncallName;
};


// The RocksCachelibWrapper is a concrete implementation of
// rocksdb::SecondaryCache. It can be allocated using
// NewRocksCachelibWrapper() and the resulting pointer
// can be passed in rocksdb::LRUCacheOptions to
// rocksdb::NewLRUCache().
//
// Users can also cast a pointer to it and call methods on
// it directly, especially custom methods that may be added
// in the future.  For example -
// std::unique_ptr<rocksdb::SecondaryCache> cache =
//      NewRocksCachelibWrapper(opts);
// static_cast<RocksCachelibWrapper*>(cache.get())->Erase(key);
class RocksCachelibWrapper : public rocksdb::SecondaryCache {
 public:
  RocksCachelibWrapper(FbCacheConfig& config, unsigned long flash_cache_size, bool enable_cache_filter, bool enable_admission_queue, bool enable_replacement) {
            if (enable_cache_filter) {
                uint64_t total_item = flash_cache_size * 1024UL / 4;
                cacheFilter_ = new CacheFilter(total_item);
                // callback function
                auto itemDestructor = [&](const DestructorData& data) {
                    if(data.context == DestructorContext::kEvictedFromNVM) {
                        FbCacheKey key = data.item.getKey();
                        cacheFilter_->Remove(key);
                    }
                };
                config.setItemDestructor(itemDestructor);
            }

            if (enable_admission_queue) {
                uint64_t queue_size = 10;
                admissionQueue_ = new AdmissionQueue(queue_size);
            }

            std::unique_ptr<FbCache> cache = std::make_unique<FbCache>(config);
            pool_ = cache->addPool("default", cache->getCacheMemoryStats().cacheSize);
            cache_ = std::move(cache).release();
        }
  ~RocksCachelibWrapper() override;

  const char* Name() const override { return "RocksCachelibWrapper"; }

  rocksdb::Status Insert(const rocksdb::Slice& key,
                         void* value,
                         const rocksdb::Cache::CacheItemHelper* helper) override;
                
  rocksdb::Status KeyRangePrefetch(std::string db_name, std::vector<char> smallest_data, std::vector<char> largest_data, int32_t level, std::shared_ptr<rocksdb::SecondaryCache> secondary_cache) override;

  rocksdb::Status RecordInvalidFiles(const std::string& db_name, const std::vector<uint64_t>& files) override;

  rocksdb::Status EvictInvalidBlocks(const std::vector<rocksdb::CacheKey>& blocks) override;

  std::unique_ptr<rocksdb::SecondaryCacheResultHandle> Lookup(
      const rocksdb::Slice& key,
      const rocksdb::Cache::CreateCallback& create_cb,
      bool wait,
      bool advise_erase,
      bool& is_in_sec_cache) override;

  bool SupportForceErase() const override { return false; }

  void Erase(const rocksdb::Slice& key) override;

  void WaitAll(
      std::vector<rocksdb::SecondaryCacheResultHandle*> handles) override;

  // TODO
  std::string GetPrintableOptions() const override { return ""; }

  // Calling Close() persists the cachelib state to the file and frees the
  // cachelib object. After calling Close(), subsequent lookups will fail,
  // and subsequent inserts will be silently ignored. Close() is not thread
  // safe, i.e only one thread can call it at a time. It doesn't require
  // ongoing lookups and inserts by other threads to be quiesced.
  void Close();

 private:
  std::atomic<FbCache*> cache_;
  cachelib::PoolId pool_;
  CacheFilter* cacheFilter_;
  AdmissionQueue* admissionQueue_;
};

// Allocate a new Cache instance with a rocksdb::TieredCache wrapper around it
extern std::shared_ptr<rocksdb::SecondaryCache> NewRocksCachelibWrapper(unsigned long flash_cache_size, bool enable_cache_filter, bool enable_admission_queue, bool enable_replacement);
} // namespace rocks_secondary_cache
} // namespace facebook