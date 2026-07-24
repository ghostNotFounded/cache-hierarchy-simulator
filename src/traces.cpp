#include "traces.hpp"

#include "cache.hpp"
#include "globals.hpp"

TraceHandler::TraceHandler(const std::string& traceFile)
    : traceFile(traceFile) {}

void TraceHandler::readTraces() {
    std::ifstream traceFileStream(traceFile);
    if (!traceFileStream.is_open()) {
        std::cerr << "Error: Could not open trace file '" << traceFile
                  << "'.\n";
        exit(1);
    }

    std::string line;
    while (std::getline(traceFileStream, line)) {
        char operation;
        std::string address;
        int instructionsSinceLastAccess;

        std::istringstream iss(line);
        iss >> operation >> address >> instructionsSinceLastAccess;

        TraceOperation opType;

        if (std::tolower(operation) == 'l') {
            opType = TraceOperation::READ;
        } else if (std::tolower(operation) == 's') {
            opType = TraceOperation::WRITE;
        } else {
            std::cerr << "Error: Invalid operation '" << operation
                      << "' in trace file.\n";
            exit(1);
        }

        if (hierarchy) {
            std::vector<int> stats = hierarchy->handleTrace(
                opType, address, instructionsSinceLastAccess);

            totalLoads += stats[0];
            totalStores += stats[1];
            loadHits += stats[2];
            loadMisses += stats[3];
            storeHits += stats[4];
            storeMisses += stats[5];
            totalCycles += stats[6];
        } else {
            std::cerr << "Error: Cache hierarchy is not initialized.\n";
            exit(1);
        }
    }
}