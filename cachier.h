#ifndef CACHIER_H
#define CACHIER_H

#include <algorithm>
#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

/**
 * @brief The Cachier class
 * @author Keshav Bhatt <keshavnrj@gmail.com>
 */
class Cachier {
public:
  /**
   * @brief The CacheOverwriteOption enum
   */
  enum CacheOverwriteOption { OVERWRITE_CACHE, DONT_OVERWRITE_CACHE };

  /**
   * HashResult holds the hash key and error string
   * @brief The HashResult class
   */
  struct HashResult {
    std::size_t key;
    std::string error;
  };

  /**
   * Instantiate Cachier object using provided cache store path.
   *
   * @brief Cachier
   * @param cache_store_path The directory path where cache will be stored.
   * @param ensure_cache_store_path Try to create cache store path during
   * cachier object instantiation, default true.
   */
  Cachier(const std::string &cache_store_path,
          bool ensure_cache_store_path = true)
      : cache_store_path(cache_store_path) {

    // Create cache_store_path if asked
    if (ensure_cache_store_path) {
      createDir(cache_store_path);
    }

    // Check if the cache_store_path is valid and writable
    struct stat dir_stat;
    if (!(stat(cache_store_path.c_str(), &dir_stat) == 0 &&
          S_ISDIR(dir_stat.st_mode) &&
          access(cache_store_path.c_str(), W_OK) == 0)) {
      std::cerr
          << "Warning: Cache store path is not valid or not writeable, caching "
             "feature will not work."
          << std::endl;
    } else {
      initialized = true;
    }
  }

  /**
   * Compute {@link HashResult} for file and create a cache entry with given
   * content in cache_store_path.
   *
   * @brief addCache
   * @param filename The complete file path to which the cache needs to be
   * added.
   * @param content Optional content or computed data to store in the cache
   * file.
   * @param cache_overwrite_option {@link CacheOverwriteOption} option whether
   * overwrite content if cache entry already exists, Default is {@link
   * CacheOverwriteOption::DONT_OVERWRITE_CACHE}
   * @return The computed {@link HashResult} object.
   */
  HashResult addCache(const std::string &filename,
                      const std::string &content = "",
                      const CacheOverwriteOption &cache_overwrite_option =
                          DONT_OVERWRITE_CACHE) {

    initCheck();

    // Compute hash
    HashResult hash_result = computeHash(filename);

    if (hash_result.error != "") {
      return {0, hash_result.error};
    }

    // Prevent overwrite if cache exists and was asked not to overwrite it
    if (cache_overwrite_option == DONT_OVERWRITE_CACHE &&
        cacheExists(hash_result.key)) {
      auto error = "Error: cache exists, not over-writing it.";
      return {0, error};
    }

    // Add file to cache_store_path
    bool cache_created =
        createCacheFile(std::to_string(hash_result.key), content);

    HashResult error = {0, "Error: unable to create cache file."};
    return cache_created ? hash_result : error;
  }

  /**
   * Return whether cache exists for the given hash key.
   *
   * @brief cacheExists
   * @param The hash key for cache entry.
   * @return True if cache exists for provided key, false otherwise.
   */
  bool cacheExists(const std::size_t &key) {
    // Check if file is in cache_store_path
    return fileExists(cache_store_path +
                      std::filesystem::path::preferred_separator +
                      std::to_string(key));
  }

  /**
   * Retunrs whether cache exists for the given filename.
   * Computes hash using file data and look for a file entry in
   * cache_store_path, return true if cache entry found indicating file is
   * cached, otherwise false.
   *
   * @brief cacheExists
   * @param filename The filename for which cache presense is being checked.
   * @return True if file exists in cache store path.
   */
  bool cacheExists(const std::string &filename) {

    initCheck();

    // Compute hash
    auto hash_result = computeHash(filename);

    // Check if file is in cache_store_path
    return fileExists(cache_store_path +
                      std::filesystem::path::preferred_separator +
                      std::to_string(hash_result.key));
  }

  /**
   * Returns the state of library initialization.
   *
   * @brief isInitialized
   * @return True if initialized, false otherwise.
   */
  bool isInitialized() {
    initialization_checked = true;
    return initialized;
  }

  /**
   * Return a {@link HashResult} object containing computed hash from file data
   * and error if any, The hash is computed using these file stats:
   *  - File name
   *  - File size
   *  - Last modification time
   *  - First 8 bytes of file aka. file header info.
   *
   * @brief computeHash
   * @param filename The File path for which the hash to be computed.
   * @return The computed {@link HashResult} object.
   */
  HashResult computeHash(const std::string &filename) {

    struct stat fileStat;

    if (stat(filename.c_str(), &fileStat) != 0 &&
        !std::filesystem::is_regular_file(filename)) {
      auto error = "Error: " + filename + " is not a valid file.";
      return {0, error};
    }

    // Get file size and modification time
    size_t fileSize = fileStat.st_size;
    time_t fileTime = fileStat.st_mtime;

    // Read first 8 bytes of file
    std::ifstream file(filename, std::ios::binary);
    std::vector<char> buffer(8);
    file.read(buffer.data(), 8);

    // Hash file name, size, time, and header info
    std::string fileHeader(buffer.begin(), buffer.end());
    std::stringstream ss;
    ss << fileSize << fileTime << fileHeader;
    std::string fileString = filename + ss.str();
    std::size_t fileHash = std::hash<std::string>{}(fileString);

    return {fileHash, ""};
  }

  /**
   * Returns data stored in cache for provided key.
   *
   * @brief getContent
   * @param key The hash key to use to get content.
   * @return The content string stored with key.
   */
  std::string getContent(const std::string &key) {
    auto target_cache_file_path =
        cache_store_path + std::filesystem::path::preferred_separator + key;
    return read_file_content(target_cache_file_path);
  }

private:
  std::string cache_store_path;

  bool initialized = false;

  bool initialization_checked = false;

  /**
   * @brief stringToSize_t
   * @param str
   * @return
   */
  std::size_t stringToSize_t(std::string str) {
    std::stringstream sstream(str);
    size_t result;
    sstream >> result;
    return result;
  }

  /**
   * @brief createCacheFile
   * @param filename
   * @param content
   * @return
   */
  bool createCacheFile(const std::string &filename,
                       const std::string &content = "") {
    std::string file_path = cache_store_path +
                            std::filesystem::path::preferred_separator +
                            filename;

    std::ofstream file(file_path);
    file << content;
    file.close();

    return file.good();
  }

  /**
   * Checks if the given file path corresponds to an existing file or
   * directory.
   *
   * @brief fileExists
   * @param path
   * @return true if the file or directory specified by path exists, false
   * otherwise.
   */
  bool fileExists(const std::string &path) {
    return std::filesystem::exists(path);
  }

  /**
   * Creates a directory at the specified path using.
   * If the directory already exists, the function returns false.
   *
   * @param path The path of the directory to create.
   * @return true if the directory was created successfully, false otherwise.
   */
  bool createDir(const std::string &path) {
    if (!std::filesystem::exists(path))
      return std::filesystem::create_directories(path);
    else
      return false;
  }

  /**
   * Reads the contents of the file and returns it as string.
   * If the file cannot be opened, an empty string is returned.
   *
   * @param filename The name of the file to read.
   * @return The content of the file as string, or an empty string if the
   * file cannot be opened.
   */
  std::string read_file_content(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      // handle error
      return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)),
                        (std::istreambuf_iterator<char>()));
    return content;
  }

  /**
   * Library initialization checks.
   *
   * @brief initCheck
   */
  void initCheck() {
    if (initialization_checked == false) {
      throw std::runtime_error("Initialization checks were not performed, did "
                               "you forgot calling Cachier::isInitialized "
                               "before using the library?");
    }
  }
};

#endif // CACHIER_H
