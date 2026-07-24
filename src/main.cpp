#include "cache.hpp"
#include "globals.hpp"
#include "traces.hpp"

cacheHierarchy* hierarchy = nullptr;
int bitsInAddress = 0;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <traces>.trace\n";
        return 1;
    }

    std::string traceFile = std::string(argv[1]);

    if (!traceFile.ends_with(".trace")) {
        std::cout << "Usage: " << argv[0] << " <traces>.trace\n";
        return 1;
    }

    try {
        auto config = toml::parse_file("config.toml");
        bitsInAddress = config["address_bits"].value_or(32);

        hierarchy = new cacheHierarchy(config);

        std::cout << "Successfully created "
                  << hierarchy->getCacheLevels().size() << " cache level(s).\n";

    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    TraceHandler traceHandler(traceFile);
    traceHandler.readTraces();

    std::cout << "Total loads:      " << traceHandler.getTotalLoads()
              << std::endl;
    std::cout << "Total stores:     " << traceHandler.getTotalStores()
              << std::endl;
    std::cout << "Load hits:        " << traceHandler.getLoadHits()
              << std::endl;
    std::cout << "Load misses:      " << traceHandler.getLoadMisses()
              << std::endl;
    std::cout << "Store hits:       " << traceHandler.getStoreHits()
              << std::endl;
    std::cout << "Store misses:     " << traceHandler.getStoreMisses()
              << std::endl;
    std::cout << "Total cycles:     " << traceHandler.getTotalCycles()
              << std::endl;
    return 0;
}