#include "cache.hpp"

cacheHierarchy::cacheHierarchy(const toml::table& config) {
    for (int levelNum = 1;; ++levelNum) {
        std::string section = "L" + std::to_string(levelNum);
        auto* table = config[section].as_table();

        if (!table) {
            if (levelNum == 1) {
                throw std::runtime_error(
                    "Configuration Error: L1 cache configuration is missing.");
            }

            std::string nextSection = "L" + std::to_string(levelNum + 1);
            if (config[nextSection]) {
                throw std::runtime_error(
                    "Configuration Error: Found '" + nextSection + "' but '" +
                    section + "' is missing. Cache levels must be contiguous.");
            }
            break;
        }

        addCacheLevel(*table);
    }
}

void cacheHierarchy::addCacheLevel(const toml::table& cacheConfig) {
    cacheLevels.emplace_back(cacheConfig);
}

void cacheLevel::updateTimestamp(int instructionsSinceLastAccess) {
    for (auto& set : cacheSets) {
        for (auto& block : set) {
            if (block.valid) {
                block.timestamp += instructionsSinceLastAccess;
            }
        }
    }
}

std::vector<int> cacheHierarchy::handleTrace(TraceOperation op,
                                             const std::string& addressStr,
                                             int instructionsSinceLastAccess) {
    uint64_t address = std::stoull(addressStr, nullptr, 16);

    auto toBinaryString = [](uint64_t value, int numBits) {
        if (numBits <= 0) return std::string("0");
        std::string binary = "";
        for (int i = numBits - 1; i >= 0; i--) {
            binary += ((value >> i) & 1) ? '1' : '0';
        }
        return binary;
    };

    updateBlockTimestamps(instructionsSinceLastAccess);

    bool found = false;
    int cycles = 0;
    for (auto& level : cacheLevels) {
        uint64_t tempAddr = address;

        int offsetBits = level.getBlockOffsetBits();
        uint64_t blockOffset = tempAddr & ((1ULL << offsetBits) - 1);
        tempAddr >>= offsetBits;

        int indexBits = level.getSetIndexBits();
        uint64_t index = tempAddr & ((1ULL << indexBits) - 1);
        tempAddr >>= indexBits;

        int tagBits = 32 - offsetBits - indexBits;
        uint64_t tag = tempAddr & ((1ULL << tagBits) - 1);

        auto& set = level.getCache()[index];
        for (auto& block : set) {
            if (block.valid && block.tag == tag) {
                found = true;
                if (op == TraceOperation::WRITE) block.dirty = true;

                break;
            }
        }

        cycles += level.getCycles();
        if (found) break;
    }

    if (!found) {
        int bytesPerBlock = cacheLevels.front().getBytesPerBlock();
        int chunksToTransfer = bytesPerBlock / 4;
        cycles += chunksToTransfer * processorCyclesForMM;

        auto& level = cacheLevels.front();
        int index = (address >> level.getBlockOffsetBits()) &
                    ((1ULL << level.getSetIndexBits()) - 1);
        uint64_t tag =
            address >> (level.getBlockOffsetBits() + level.getSetIndexBits());

        bool evictedDirty = level.setCacheBlock(index, tag, op);
        if (evictedDirty) {
            cycles += chunksToTransfer * processorCyclesForMM;
        }
    }

    return {op == TraceOperation::READ,               // Load operation
            op == TraceOperation::WRITE,              // Store operation
            found && (op == TraceOperation::READ),    // Load hit
            !found && (op == TraceOperation::READ),   // Load miss
            found && (op == TraceOperation::WRITE),   // Store hit
            !found && (op == TraceOperation::WRITE),  // Store miss
            cycles};
}

// Cache level class
cacheLevel::cacheLevel(const toml::table& cacheConfig) {
    level = cacheConfig["level"].value_or(std::string{"UNKNOWN"});
    cycles = cacheConfig["cycles"].value_or(1);

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

    blockOffsetBits = log2(bytesPerBlock);
    setIndexBits = log2(setsInCache);
    tagBits = bitsInAddress - blockOffsetBits - setIndexBits;

    cacheSets = std::vector<std::vector<CacheBlock>>(
        setsInCache,
        std::vector<CacheBlock>(blocksPerSet, {0, false, false, 0}));
}

bool cacheLevel::setCacheBlock(int index, uint64_t tag, TraceOperation op) {
    auto& set = cacheSets[index];
    int oldestIdx = 0;

    for (size_t i = 0; i < set.size(); ++i) {
        if (!set[i].valid) {
            set[i].tag = tag;
            set[i].valid = true;
            set[i].timestamp = 0;
            if (op == TraceOperation::WRITE) set[i].dirty = true;
            return false;
        }
        if (set[i].timestamp > set[oldestIdx].timestamp) {
            oldestIdx = i;
        }
    }

    set[oldestIdx].tag = tag;
    set[oldestIdx].valid = true;
    set[oldestIdx].dirty = (op == TraceOperation::WRITE);
    set[oldestIdx].timestamp = 0;

    return set[oldestIdx].dirty;
}