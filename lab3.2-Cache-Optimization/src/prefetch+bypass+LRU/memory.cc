#include "memory.h"

void Memory::HandleRequest(uint64_t addr, int bytes, int read,
                          char *content, int &hit, int &cycle) {
	// Simply add the latency to cycle
	cycle += latency_.hit_latency + latency_.bus_latency;
}

