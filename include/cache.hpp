#pragma once
#include <iostream>
#include <toml.hpp>

enum evictionPolicies { LRU, FIFO };

class cacheLevel {
 public:
  cacheLevel(const toml::table& cacheConfig);

  std::string getCacheLevel() const;
  int getSetsInCache() const;
  int getBlocksPerSet() const;
  int getBytesPerBlock() const;

  bool getWriteAllocate() const;
  bool getWriteThrough() const;

  evictionPolicies getEvictionPolicy() const;

 private:
  std::string level;

  int setsInCache;
  int blocksPerSet;
  int bytesPerBlock;
  bool writeAllocate;
  bool writeThrough;
  evictionPolicies eviction;
};