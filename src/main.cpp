#include <bit>
#include <iostream>
#include <stdexcept>
#include <string>

#include "cache.hpp"

int main() {
  try {
    auto config = toml::parse_file("config.toml");

    std::vector<cacheLevel> cacheHierarchy;

    for (int level = 1;; ++level) {
      std::string section = "L" + std::to_string(level);

      auto* table = config[section].as_table();

      // Missing cache level
      if (!table) {
        if (level == 1) {
          throw std::runtime_error(
              "Configuration Error: L1 cache configuration is missing.");
        }

        // Check if any later level exists
        std::string nextSection = "L" + std::to_string(level + 1);

        if (config[nextSection]) {
          throw std::runtime_error(
              "Configuration Error: Found '" + nextSection + "' but '" +
              section + "' is missing. Cache levels must be contiguous.");
        }

        break;
      }

      cacheHierarchy.emplace_back(*table);
    }

    std::cout << "Successfully created " << cacheHierarchy.size()
              << " cache level(s).\n";

  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
  }

  return 0;
}
