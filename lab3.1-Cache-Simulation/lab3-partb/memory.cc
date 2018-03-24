#include "memory.h"
#include <cstring>
void Memory::HandleRequest(uint64_t addr, int bytes, int read,
                          char *content, int &hit, int &cycle) 
{
	// Simply add the latency to cycle
	if(read == 1)//read
	{
		memcpy(content, mem_space+addr, bytes);
	}
	else if(read == 0)//write
	{
		memcpy(mem_space+addr, content, bytes);
	}
	cycle += latency_.hit_latency + latency_.bus_latency;
}
char* Memory::GetMemAddr()
{
	return mem_space;
}
