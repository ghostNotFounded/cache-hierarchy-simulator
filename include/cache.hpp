#pragma once
#include <iostream>
#include <toml.hpp>

enum evictionPolicies { LRU, FIFO };

class cacheLevel {
 public:
  cacheLevel(const toml::table& cacheConfig);

 private:
  std::string level;

  int setsInCache;
  int blocksPerSet;
  int bytesPerBlock;
  bool writeAllocate;
  bool writeThrough;
  evictionPolicies eviction;
};