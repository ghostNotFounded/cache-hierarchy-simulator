#include "cache.hpp"

cacheLevel::cacheLevel(const toml::table& cacheConfig) {
  level = cacheConfig["level"].value_or(std::string{"UNKNOWN"});

  setsInCache = cacheConfig["sets_in_cache"].value_or(0);
  if (setsInCache <= 0 ||
      !std::__has_single_bit(static_cast<unsigned>(setsInCache)))
    throw std::runtime_error(
        "Configuration Error [" + level +
        ".sets_in_cache]: expected a positive power of two.");

  blocksPerSet = cacheConfig["blocks_per_set"].value_or(0);
  if (blocksPerSet <= 0 ||
      !std::__has_single_bit(static_cast<unsigned>(blocksPerSet)))
    throw std::runtime_error(
        "Configuration Error [" + level +
        ".blocks_per_set]: expected a positive power of two.");

  bytesPerBlock = cacheConfig["bytes_per_block"].value_or(0);
  if (bytesPerBlock < 4 ||
      !std::__has_single_bit(static_cast<unsigned>(bytesPerBlock)))
    throw std::runtime_error(
        "Configuration Error [" + level +
        ".bytes_per_block]: expected a power of two >= 4.");

  writeAllocate = cacheConfig["write_allocate"].value_or(true);
  writeThrough = cacheConfig["write_through"].value_or(true);

  std::string evictionStr = cacheConfig["eviction"].value_or(std::string{});

  if (evictionStr == "lru" || evictionStr == "LRU")
    eviction = LRU;
  else if (evictionStr == "fifo" || evictionStr == "FIFO")
    eviction = FIFO;
  else
    throw std::runtime_error("Configuration Error [" + level +
                             ".eviction]: expected 'lru' or 'fifo'.");
}