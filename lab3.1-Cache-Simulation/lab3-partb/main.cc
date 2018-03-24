#include "stdio.h"
#include "cache.h"
#include "memory.h"
#include <fstream>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
	if (argc < 2) {
		cout << "argument lost.";
		return 0;
	}
	else if (argc > 2) {
		cout << "too much arguments";
		return 0;
	}
	string file(argv[1]);
	int size;				// Cache's size (in bytes)
	int associativity;		// associativity of the cache
	int set_num;			// Number of cache sets
	int write_through;		// 0|1 for back|through
	int write_allocate;		// 0|1 for no-alc|alc

	StorageLatency memory_lantency;	// Store the lantency of the memory
	memory_lantency.bus_latency = 0;	// The latency of accessing the memory is pre-defined
	memory_lantency.hit_latency = 100;

	StorageLatency cache_latency;	// Store the lantency of the cache (the data needs )
	cache_latency.bus_latency = 0;	// bus_latency is always 0

	cout << "Size of cache (in byte): ";
	cin >> size;
	cout << "Associativity: ";
	cin >> associativity;
	cout << "Number of line size: ";
	int line_size;
	cin >> line_size;
	set_num=size / (associativity * line_size);
	cout << "Write hit policy: (0|1 for back|through)";
	cin >> write_through;
	cout << "Write missing policy: (0|1 for no-alc|alc)";
	cin >> write_allocate;
	cout << "Cache hit latency (in ns): ";
	double tmp;
	cin >> tmp;
	tmp = tmp * 2;
	int latency = tmp;
	if ((double)latency < tmp)
		latency++;
	cache_latency.hit_latency = latency;

	// Initialize the CacheConfig
	CacheConfig stat(size, set_num, associativity, write_through, write_allocate);

	Memory memory;
	Cache cache;
	// Initialize the stat of cache
	cache.SetConfig(stat);

	// Initialize the latency of memory & cache
	memory.SetLatency(memory_lantency);
	cache.SetLatency(cache_latency);

	// hit is used to count the number of data-hitting
	int hit = 0;
	int count = 0;
	// time is used to count the time (in cycles)
	int time = 0;

	char content[64];
	unsigned long long addr;
	char method;
	ifstream fin;
	fin.open(argv[1]);

	// Set the memory as the lower level of cache
	cache.SetLower(&memory);
	// Main access process
	// [in]  addr: access address
	// [in]  bytes: target number of bytes
	// [in]  read: 0|1 for write|read
	//             3|4 for write|read in prefetch
	// [i|o] content: in|out data
	// [out] hit: 0|1 for miss|hit
	// [out] cycle: total access cycle

	while (fin>>method>>addr)
	{
		count++;
		if (method == 'w')
			cache.HandleRequest(addr, 0, 0, content, hit, time);
		else
			cache.HandleRequest(addr, 0, 1, content, hit, time);
	}

	cout << "Hit times: " << hit << endl;
	cout << "Access count: " << count << endl;
	cout << "Hit rate: " << ((double)hit) / ((double)count) << endl;
	cout << "Miss rate: " << 1 - (((double)hit) / ((double)count)) << endl;
	cout << "Request access time: (in cycle)" << time << endl;
	fin.close();

	return 0;
}
