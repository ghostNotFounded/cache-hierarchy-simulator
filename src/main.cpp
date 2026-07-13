#include <iostream>
#include <string>
#include <stdexcept>
#include <toml.hpp>
#include <bit>

enum evictionPolicies {
    LRU,
    FIFO
};

class cacheLevel {
private:

public:
    int setsInCache;
    int blocksPerSet;
    int bytesPerBlock;
    bool writeAllocate;
    bool writeThrough;
    evictionPolicies eviction;

    cacheLevel() {
        auto config = toml::parse_file("config.toml");
        auto l1 = config["L1"];

        setsInCache = l1["sets_in_cache"].value_or(0);
        if (setsInCache <= 0 || !std::__has_single_bit(static_cast<unsigned>(setsInCache)))
            throw std::runtime_error(
                "Configuration Error [L1.sets_in_cache]: expected a positive power of two (1, 2, 4, 8, ...). "
                "The cache likes symmetry."
            );

        blocksPerSet = l1["blocks_per_set"].value_or(0);
        if (blocksPerSet <= 0 || !std::__has_single_bit(static_cast<unsigned>(blocksPerSet)))
            throw std::runtime_error(
                "Configuration Error [L1.blocks_per_set]: expected a positive power of two. "
                "Associativity has standards."
            );

        bytesPerBlock = l1["bytes_per_block"].value_or(0);
        if (bytesPerBlock < 4 || !std::__has_single_bit(static_cast<unsigned>(bytesPerBlock)))
            throw std::runtime_error(
                "Configuration Error [L1.bytes_per_block]: expected a power of two with a minimum value of 4 bytes. "
                "Tiny cache lines are on strike."
            );

        writeAllocate = l1["write_allocate"].value_or(true); 
        writeThrough = l1["write_through"].value_or(true);

        std::string evictionStr = l1["eviction"].value_or(std::string{});

        if (evictionStr == "lru" || evictionStr == "LRU")
            eviction = LRU;
        else if (evictionStr == "fifo" || evictionStr == "FIFO")
            eviction = FIFO;
        else
            throw std::runtime_error(
                "Configuration Error [L1.eviction]: unknown eviction policy '" + evictionStr +
                "'. Supported values are 'lru' and 'fifo'. No blocks were harmed... yet."
            );
            }
};

int main() {
    try {
        cacheLevel L1Cache = cacheLevel();
    }
    catch(const std::exception& e) {
        std::cerr << "Ran into an error: " << e.what() << '\n';
        return 1;
    }
    
    return 0;
}
