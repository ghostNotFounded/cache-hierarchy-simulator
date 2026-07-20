#pragma once

#include <bit>
#include <iostream>
#include <string>
#include <toml.hpp>
#include <vector>

#include "traces.hpp"

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

class cacheHierarchy {
   public:
    cacheHierarchy() = default;
    cacheHierarchy(const toml::table& config);

    void addCacheLevel(const toml::table& cacheConfig);
    void handleTrace(TraceOperation op, const std::string& address,
                     int instructionsSinceLastAccess);

    const std::vector<cacheLevel>& getCacheLevels() const;

   private:
    std::vector<cacheLevel> cacheLevels;
};