#include "cache.hpp"
#include "globals.hpp"
#include "traces.hpp"

cacheHierarchy* hierarchy = nullptr;

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
        hierarchy = new cacheHierarchy(config);

        std::cout << "Successfully created "
                  << hierarchy->getCacheLevels().size() << " cache level(s).\n";

    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    TraceHandler traceHandler;
    traceHandler.readTraces(traceFile);

    return 0;
}