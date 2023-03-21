# Cachier

Cachier is a fast C++ header only library that provides simple and efficient way to cache computed data for files.

**How it works**

1. Initialize Cachier with a `cache_path`, which is the directory where you want to store your cache.
2. Process or extract some data from a file. (ex: Amplitude data from a big audio file) 
3. Store the computed data to the cache using Cachier's caching method.
4. Cachier computes a `unique key` for the file using its properties, like the `file name`, `size`, `last modification time`, and the first 8 bytes of file `header info`.
5. The computed data is saved to the provided `cache_path` as a file named using the unique key computed by Cachier.
6. Later on, retrieve the computed data for the same file by asking Cachier to find it in the cache.
7. Cachier computes the key for the file using the same strategy as before.
8. Cachier performs a lookup in the cache directory for the file named using the computed key and returns the data stored in it. Note that the cache entry becomes invalid if the file has been modified.


**Use Case**

Suppose you're developing an audio editor application where you need to perform certain calculations on an audio file each time you open it for editing. This process can take a significant amount of time and resources. To optimize your application, you can use Cachier to store the computed data in cache and retrieve it later whenever you open the same file again.

With Cachier, you can rely on the cached data to be correct and up-to-date, without the need to re-calculate it each time the file is opened. This results in a more efficient and faster user experience for your audio editor application.

**How to use it**

1. Clone the repo or just somehow get and the [cachier.h](https://github.com/keshavbhatt/Cachier/blob/main/cachier.h) file in your project
2. Read how to use the library [here](https://keshavbhatt.github.io/Cachier/), you can find some usages in [main.cpp](https://github.com/keshavbhatt/Cachier/blob/main/main.cpp) 
