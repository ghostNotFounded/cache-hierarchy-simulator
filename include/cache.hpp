#pragma once

#include <bit>
#include <iostream>
#include <string>
#include <toml.hpp>
#include <vector>

#include "globals.hpp"
#include "traces.hpp"

enum evictionPolicies { LRU, FIFO };

struct CacheBlock {
    uint64_t tag;
    bool dirty;
    bool valid;
    int timestamp;
};

class cacheLevel {
   public:
    cacheLevel(const toml::table& cacheConfig);

    std::string getCacheLevel() const;
    std::vector<std::vector<CacheBlock>>& getCache() { return cacheSets; };

    void updateTimestamp(int instructionsSinceLastAccess);

    bool setCacheBlock(int index, uint64_t tag, TraceOperation op);

    int getBlockOffsetBits() const { return blockOffsetBits; }
    int getSetIndexBits() const { return setIndexBits; }
    int getTagBits() const { return tagBits; }

    int getSetsInCache() const { return setsInCache; }
    int getBlocksPerSet() const { return blocksPerSet; }
    int getBytesPerBlock() const { return bytesPerBlock; }

    bool getWriteAllocate() const { return writeAllocate; }
    bool getWriteThrough() const { return writeThrough; }

    int getCycles() const { return cycles; }

    evictionPolicies getEvictionPolicy() const { return eviction; }

   private:
    std::string level;
    std::vector<std::vector<CacheBlock>> cacheSets;

    int blockOffsetBits;
    int setIndexBits;
    int tagBits;

    int cycles;

    int setsInCache;
    int blocksPerSet;
    int bytesPerBlock;
    bool writeAllocate;
    bool writeThrough;
    evictionPolicies eviction;
};

class cacheHierarchy {
   public:
    cacheHierarchy(const toml::table& config);
    ~cacheHierarchy() = default;

    void updateBlockTimestamps(int instructionsSinceLastAccess) {
        for (auto& level : cacheLevels) {
            level.updateTimestamp(instructionsSinceLastAccess);
        }
    }

    void addCacheLevel(const toml::table& cacheConfig);
    std::vector<int> handleTrace(TraceOperation op, const std::string& address,
                                 int instructionsSinceLastAccess);

    const std::vector<cacheLevel>& getCacheLevels() const {
        return cacheLevels;
    }

   private:
    std::vector<cacheLevel> cacheLevels;
};