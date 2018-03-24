#ifndef CACHE_MEMORY_H_
#define CACHE_MEMORY_H_

#include <stdint.h>
#include "storage.h"
#include <cstring>
#define LEN 400000000
class Memory: public Storage {
  char mem_space[LEN];
 public:
  Memory() 
{
	memset(mem_space, 0, sizeof(mem_space));
}
  ~Memory() {}

  // Main access process
  void HandleRequest(uint64_t addr, int bytes, int read,
                     char *content, int &hit, int &cycle);
  char* GetMemAddr();
/*
 private:
  // Memory implement

  DISALLOW_COPY_AND_ASSIGN(Memory);*/
};

#endif //CACHE_MEMORY_H_ 
