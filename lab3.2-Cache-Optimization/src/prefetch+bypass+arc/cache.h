#ifndef CACHE_CACHE_H_
#define CACHE_CACHE_H_

#include "storage.h"
#include <stdint.h>

#define LRU     0
#define LFU     1
#define FIFO    2
#define RANDOM  3

#define TAG(addr, s, b)          ( addr>>(s+b) )
#define SET_INDEX(addr, s, b)    ( (addr>>b) & ((uint64_t(1)<<s)-1) )
#define BLOCK_INDEX(addr, s, b)  ( addr & ((uint64_t(1)<<b)-1) )

typedef struct CacheConfig_ {
  CacheConfig_(){}
  // Initialize the data
  CacheConfig_(int a,int b,int c,int d,int e,int f, int g)
  {
    size = a;
    set_num = b;
    associativity = c;
    write_through = d;
    write_allocate = e;
    prefetch_num = f;
    bypass_policy = g;
    block_num = size / set_num / associativity;
  }
  int size;
  int set_num; // Number sets in a cache
  int associativity; // Number of lines in a set
  int block_num; // Number of blocks in a line
  int write_through; // 0|1 for back|through
  int write_allocate; // 0|1 for no-alc|alc
  int prefetch_num;
  int bypass_policy;
} CacheConfig;

/*
 * Line - a line: contains valid, tag, dirty, block[], in addition to last_access to implement LRU
 */
typedef struct Line_ {
	unsigned valid;
	unsigned tag;
	unsigned dirty;
	unsigned last_access;
	unsigned access_count;
	unsigned first_access;
	unsigned prefetch_tag;
	unsigned char* block;
} Line;

/*
 * Set - a set: contains some lines, with tag_map to check for hit/miss, and LRU_map to find out victim
 */
typedef struct Set_ {
	int lru_num;
	int lfu_num;
	int lru_gost_num;
	int lfu_gost_num;
	Line *line;
} Set;

class Cache: public Storage {
 public:
  Cache() {}
  ~Cache() {}

  // Sets & Gets
  void SetConfig(CacheConfig cc);
  void GetConfig(CacheConfig &cc);
  void SetLower(Storage *ll) { lower_ = ll; }
  // Main access process
  void HandleRequest(uint64_t addr, int bytes, int read,
                     char *content, int &hit, int &cycle);
  int LRU_line(int set_index, int& if_free_line);

 private:
  // Bypassing
  int BypassDecision();
  // Partitioning
  void PartitionAlgorithm();
  // Replacement
  int ReplaceDecision(uint64_t addr, uint64_t &line_index);
  void ReplaceAlgorithm(uint64_t addr, int bytes, int read,
                     char *content, int &hit, int &cycle);

  // Prefetching
  int PrefetchDecision(uint64_t addr, int hit);
  void PrefetchAlgorithm(uint64_t addr, int hit);

  Set *set;
  uint64_t access_count;
  CacheConfig config_;
  Storage *lower_; // lower storage
  DISALLOW_COPY_AND_ASSIGN(Cache);
};

#endif //CACHE_CACHE_H_
