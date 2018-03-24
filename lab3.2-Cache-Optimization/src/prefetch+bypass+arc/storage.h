#ifndef CACHE_STORAGE_H_
#define CACHE_STORAGE_H_

#include <stdint.h>
#include <stdio.h>

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&); \
  void operator=(const TypeName&)

// Storage access stats
typedef struct StorageStats_ {
  int access_counter;
  int miss_num;
  int access_cycle; 
  int replace_num; 
  int fetch_num; // Fetch lower layer
  int prefetch_num; 
  double miss_rate;
} StorageStats;

// Storage basic config
typedef struct StorageLatency_ {
  int hit_latency; // In cycles
  int bus_latency; // Added to each request
} StorageLatency;

class Storage {
 public:
  Storage() {}
  ~Storage() {}

  // Sets & Gets
  void SetStats(StorageStats ss) { stats_ = ss; }
  void GetStats(StorageStats &ss) { ss = stats_; }
  void SetLatency(StorageLatency sl) { latency_ = sl; }
  void GetLatency(StorageLatency &sl) { sl = latency_; }
  void PrintStats() {
		printf("--------------------------------\n");
		printf("The following is the stats of this cache\n");
		printf("access_counter: %d\n", stats_.access_counter);
		printf("miss_num: %d\n", stats_.miss_num);
		stats_.miss_rate = (double)stats_.miss_num/stats_.access_counter;
		printf("miss_rate: %lf\n", stats_.miss_rate);
		printf("access_cycle: %d\n", stats_.access_cycle);
		printf("replace_num: %d\n", stats_.replace_num);
		printf("fetch_num: %d\n", stats_.fetch_num);
		printf("prefetch_num: %d\n", stats_.prefetch_num);
		printf("--------------------------------\n");
  }

  // Main access process
  // [in]  addr: access address
  // [in]  bytes: target number of bytes
  // [in]  read: 0|1 for write|read
  //             2|3 for prefetch write|read
  // [i|o] content: in|out data
  // [out] hit: 0|1 for miss|hit
  // [out] cycle: total access cycle
  virtual void HandleRequest(uint64_t addr, int bytes, int read,
                             char *content, int &hit, int &cycle) = 0;

 protected:
  StorageStats stats_;
  StorageLatency latency_;
};

#endif //CACHE_STORAGE_H_ 
