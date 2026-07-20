#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

enum class TraceOperation { READ, WRITE };

class TraceHandler {
   public:
    TraceHandler() = default;
    void readTraces(const std::string& traceFile);
};