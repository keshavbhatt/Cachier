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
#include <unordered_map>
#include <vector>

/**
 * @brief The Cachier class
 * @author Keshav Bhatt <keshavnrj@gmail.com>
 */
class Cachier {
public:
  enum FileCacheOption {
    ENSURE_CACHE_STORE_PATH,
    OVERWRITE_CACHE,
    DONT_OVERWRITE_CACHE
  };

  Cachier(const std::string &cache_store_path,
          FileCacheOption ensure_cache_store_path)
      : cache_store_path(cache_store_path) {
    // Create cache_store_path if asked
    if (ensure_cache_store_path == FileCacheOption::ENSURE_CACHE_STORE_PATH) {
      createDir(cache_store_path);
    }

    // Check if thr cache_store_path is valid and writable
    struct stat st;
    if (!(stat(cache_store_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode) &&
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
   * Compute hash or key for file and create a cache entry with given content
   * in cache_store_path
   *
   * @brief addFile
   * @param filename full file path
   * @param content optional content to store in the cache file created
   * @param overwrite whether to overwrite content if cache entry already exists
   * @return hash key which was used to create cache entry
   */
  std::size_t addCacheFile(const std::string &filename,
                           const std::string &content = "",
                           FileCacheOption overwrite = DONT_OVERWRITE_CACHE) {

    initCheck();

    // Check if file exists and is a regular file
    struct stat fileStat;
    if (stat(filename.c_str(), &fileStat) == -1 || !S_ISREG(fileStat.st_mode)) {
      std::cerr << "Error: " << filename << " is not a valid file."
                << std::endl;
      return 0;
    }

    // Compute hash
    std::size_t fileHash = computeHash(filename, fileStat);

    // Prevent overwrite if cache exists and was asked not to overwrite it
    if (overwrite == DONT_OVERWRITE_CACHE && cacheExists(fileHash)) {
      return 00;
    }

    // Add file to cache_store_path
    return createCacheFile(std::to_string(fileHash), content) ? fileHash : 0;
  }

  /**
   * Check if cache exists for the given hash key
   * @brief cacheExists
   * @param hash
   * @return
   */
  bool cacheExists(const std::size_t &hash) {
    // Check if file is in cache_store_path
    return fileExists(cache_store_path +
                      std::filesystem::path::preferred_separator +
                      std::to_string(hash));
  }

  /**
   * Check cache registry and match it with computed hash using file data.
   * If hash matches and there is an entry, return true indicating file is
   * cached otherwise false
   *
   * @brief cacheExists
   * @param filename full file path
   * @return true if file exists in cache registry
   */
  bool cacheExists(const std::string &filename) {

    initCheck();

    // Check if file exists and is a regular file
    struct stat fileStat;
    if (stat(filename.c_str(), &fileStat) == -1 || !S_ISREG(fileStat.st_mode)) {
      std::cerr << "Error: " << filename << " is not a valid file."
                << std::endl;
      return false;
    }

    // Compute hash
    std::size_t fileHash = computeHash(filename, fileStat);

    // Check if file is in cache_store_path
    return fileExists(cache_store_path +
                      std::filesystem::path::preferred_separator +
                      std::to_string(fileHash));
  }

  bool isInitialized() {
    initialization_checked = true;
    return initialized;
  }

  /**
   * Create a hash from file data, composed of:
   *  - File name
   *  - File size
   *  - Last modification time
   *  - First 8 bytes of file aka. file header info
   * @brief computeHash
   * @param filename
   * @return hash value
   */
  std::size_t computeHash(const std::string &filename) {
    struct stat fileStat;
    if (stat(filename.c_str(), &fileStat) == -1 || !S_ISREG(fileStat.st_mode)) {
      std::cerr << "Error: " << filename << " is not a valid file."
                << std::endl;
      return 0;
    }
    return this->computeHash(filename, fileStat);
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
   * @brief fileExists
   * @param path
   * @return
   */
  bool fileExists(const std::string &path) {
    return std::filesystem::exists(path);
  }

  /**
   * @brief createDir
   * @param path
   * @return
   */
  bool createDir(const std::string &path) {
    if (!std::filesystem::exists(path))
      return std::filesystem::create_directories(path);
    else
      return false;
  }

  /**
   * Create a hash from file data, composed of:
   *  - File name
   *  - File size
   *  - Last modification time
   *  - First 8 bytes of file aka. file header info
   * @brief computeHash
   * @param filename
   * @param fileStat
   * @return hash value
   */
  std::size_t computeHash(const std::string &filename, struct stat fileStat) {
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

    return fileHash;
  }

  void initCheck() {
    if (initialization_checked == false) {
      throw std::runtime_error("Initialization checks were not performed, did "
                               "you forgot calling Cachier::isInitialized "
                               "before using the library?");
    }
  }
};

#endif // CACHIER_H
