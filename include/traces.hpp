#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

enum class TraceOperation { READ, WRITE };

class TraceHandler {
   public:
    TraceHandler(const std::string& traceFile);
    ~TraceHandler() = default;

    void readTraces();

    int getTotalLoads() const { return totalLoads; }
    int getTotalStores() const { return totalStores; }
    int getLoadHits() const { return loadHits; }
    int getLoadMisses() const { return loadMisses; }
    int getStoreHits() const { return storeHits; }
    int getStoreMisses() const { return storeMisses; }
    int getTotalCycles() const { return totalCycles; }

   private:
    const std::string traceFile;

    int totalLoads = 0;
    int totalStores = 0;
    int loadHits = 0;
    int loadMisses = 0;
    int storeHits = 0;
    int storeMisses = 0;
    int totalCycles = 0;
};