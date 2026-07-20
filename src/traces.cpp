#include "traces.hpp"

#include "cache.hpp"
#include "globals.hpp"

void TraceHandler::readTraces(const std::string& traceFile) {
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

        if (std::tolower(operation) == 's') {
            opType = TraceOperation::READ;
        } else if (std::tolower(operation) == 'l') {
            opType = TraceOperation::WRITE;
        } else {
            std::cerr << "Error: Invalid operation '" << operation
                      << "' in trace file.\n";
            exit(1);
        }

        if (hierarchy) {
            hierarchy->handleTrace(opType, address,
                                   instructionsSinceLastAccess);
        } else {
            std::cerr << "Error: Cache hierarchy is not initialized.\n";
            exit(1);
        }
    }
}