#include "stdio.h"
#include "cache.h"
#include "memory.h"
#include <fstream>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
	if (argc < 2 || argc > 2) {
		cout << "Please enter in the std format"<<endl;
		return 0;
	}
	int size;				
	int block_size;
	int associativity;		
	int set_num;			
	int write_through;		// 0|1 for back|through
	int write_allocate;		// 0|1 for no-alc|alc

	StorageLatency memory_lantency;	
	memory_lantency.bus_latency = 0;	
	memory_lantency.hit_latency = 100;

	StorageLatency cache_latency;	
	cache_latency.bus_latency = 0;	// bus_latency is set to zero
	cache_latency.hit_latency = 10;

	cout << "Please enter the size of cache (in KB):";
	cin >> size;
	size = size << 10;
	cout << "Please enter the block size:";
	cin >> block_size;
	cout << "Please enter the number of associativity:";
	cin >> associativity;
	set_num=size / (associativity * block_size);
	cout << "Write missing policy: (0|1 for no-alc|alc) (default policy is alc)";
	cin >> write_allocate;
	//write_allocate = 1;
	cout << "Write hit policy: (0|1 for back|through) (default policy is back)";
	cin >> write_through;
	//write_through = 0;
	cout << "Cache hit latency (in ns): (default=10)";
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
	cache.SetConfig(stat);
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
	ifstream f;
	f.open(argv[1]);

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

	while (f>>method>>addr)
	{
		count++;
		if (method == 'w')
			cache.HandleRequest(addr, 0, 0, content, hit, time);
		else
			cache.HandleRequest(addr, 0, 1, content, hit, time);
	}
	f.close();
	cout << "Access number: " << count << endl;
	cout << "Hit times: " << hit << endl;
	cout << "Miss times: " << count - hit << endl;	
	cout << "Hit rate: " << ((double)hit) / ((double)count) << endl;
	cout << "Miss rate: " << 1 - (((double)hit) / ((double)count)) << endl;
	cout << "Request access time: (in cycle)" << time << endl;

	return 0;
}
