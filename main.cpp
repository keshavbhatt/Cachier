#include "cachier.h"
#include <iostream>

using FileCacheOption = Cachier::CacheOverwriteOption;
int main() {

  std::string target_file = "/tmp/target_file";

  Cachier cache("/tmp/my_chache_store");

  if (cache.isInitialized()) {

    // add a file to the cache
    Cachier::HashResult cache_result = cache.addCache(
        target_file, "king", FileCacheOption::OVERWRITE_CACHE);

    auto key = cache_result.key;
    auto error = cache_result.error;

    if (key != 0) {
      std::cout << "File " << target_file << " cached as "
                << std::to_string(key) << std::endl;
    } else if (key == 1) {
      auto hash_result = cache.computeHash(target_file);

      std::cout << "File " + target_file + " already exists in cache as "
                << hash_result.key << std::endl;
    } else {
      std::cout << error << std::endl;
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

    bool file_in_cache = false;

    if (check_cache == "y" || check_cache.empty()) {
      file_in_cache = cache.cacheExists(target_file);
      if (file_in_cache) {
        std::cout << "File " + target_file + " is in cache!" << std::endl;
      } else {
        std::cout << "File " + target_file +
                         " is not in cache(or may have modified!)"
                  << std::endl;
      }
    }

    // test Cachier::getContent
    std::string get_content;

    std::cout << std::endl
              << "Test: getContent"
              << "Get content stored for cache entry " << cache_result.key
              << "? [y]" << std::endl;

    std::getline(std::cin, get_content);

    // convert input to lowercase
    std::transform(check_cache.begin(), check_cache.end(), check_cache.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    // get content
    if (get_content == "y" || get_content.empty()) {
      if (file_in_cache) {
        std::cout << cache.getContent(std::to_string(cache_result.key))
                  << std::endl;
      }
    }

    return 0;
  } else {
    return 1;
  }
}
