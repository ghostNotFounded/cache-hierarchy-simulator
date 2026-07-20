#include "cache.hpp"

// Cache Hierarchy class
cacheHierarchy::cacheHierarchy(const toml::table& config) {
    for (int levelNum = 1;; ++levelNum) {
        std::string section = "L" + std::to_string(levelNum);
        auto* table = config[section].as_table();

        // Missing cache level
        if (!table) {
            if (levelNum == 1) {
                throw std::runtime_error(
                    "Configuration Error: L1 cache configuration is missing.");
            }

            // Check if any later level exists
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

void cacheHierarchy::handleTrace(TraceOperation op, const std::string& address,
                                 int instructionsSinceLastAccess) {
    while (true) {
        for (auto& level : cacheLevels) {
            for (int i = 0; i < level.getSetsInCache(); i++) {
                for (int j = 0; j < level.getBlocksPerSet(); j++) {
                    std::cout << "Accessing " << address << " in "
                              << level.getCacheLevel() << "\n";
                }
            }
        }

        break;
    }
}

const std::vector<cacheLevel>& cacheHierarchy::getCacheLevels() const {
    return cacheLevels;
}

// Cache level class
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

std::string cacheLevel::getCacheLevel() const { return level; }
int cacheLevel::getSetsInCache() const { return setsInCache; }
int cacheLevel::getBlocksPerSet() const { return blocksPerSet; }
int cacheLevel::getBytesPerBlock() const { return bytesPerBlock; }

bool cacheLevel::getWriteAllocate() const { return writeAllocate; }
bool cacheLevel::getWriteThrough() const { return writeThrough; }

evictionPolicies cacheLevel::getEvictionPolicy() const { return eviction; }