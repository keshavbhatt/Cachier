#include "cachier.h"
#include <iostream>

using FileCacheOption = Cachier::FileCacheOption;
int main() {

  std::string target_file = "/tmp/target_file";

  Cachier cache("/tmp/my_chache_store",
                  FileCacheOption::ENSURE_CACHE_STORE_PATH);
  if (cache.isInitialized()) {

    // add a file to the cache
    std::size_t key = cache.addCacheFile(target_file, "king",
                                         FileCacheOption::DONT_OVERWRITE_CACHE);

    if (key != 0) {
      std::cout << "File " << target_file << " cached as "
                << std::to_string(key) << std::endl;
    } else if (key == 00) {
      auto cacheKey = cache.computeHash(target_file);

      std::cout << "File " + target_file + " already exists in cache as "
                << cacheKey << std::endl;
    } else {
      std::cout << "Failed to create cache." << std::endl;
    }

    // test Cachier::fileExists
    std::string check_cache;

    std::cout << std::endl
              << "Test: fileExists"
              << "(Modify the " << target_file
              << " to see if cache status changes)" << std::endl
              << "Check cache state for " + target_file + " file now? [y]"
              << std::endl;

    std::getline(std::cin, check_cache);

    // convert input to lowercase
    std::transform(check_cache.begin(), check_cache.end(), check_cache.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (check_cache == "y" || check_cache.empty()) {
      if (cache.cacheExists(target_file)) {
        std::cout << "File " + target_file + " is in cache!" << std::endl;
      } else {
        std::cout << "File " + target_file +
                         " is not in cache(probably modified!)"
                  << std::endl;
      }
    }
    return 0;
  } else {
    return 1;
  }
}
