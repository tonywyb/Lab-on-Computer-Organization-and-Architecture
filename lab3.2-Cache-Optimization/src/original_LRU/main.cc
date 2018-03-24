#include "stdio.h"
#include "cache.h"
#include "memory.h"
#include <fstream>
#include <iostream>
#include <stdlib.h>

using namespace std;

int main(int argc, char *argv[])
{
	// Program needs one argument as input file
	if (argc<2)
	{
		cout<<"argument lost";
		return 0;
	}
	else if(argc>2)
	{
		cout<<"too much arguments";
		return 0;
	}

	// Allocate new caches and memory
	Cache level1, level2;
	Memory memory;

	// Set the basic state of memory
	StorageStats tmpStats;
	tmpStats.access_cycle=0;
	memory.SetStats(tmpStats);

	// Set the configuration of caches, given by write-up
	// cout<<"Put in replace policy, 0|1|2|3 for LRU|LFU|FIFO|random"<<endl;
	int replace_policy = 0;
	cout<<"Replace policy: LRU"<<endl;

	// cout<<"Put in prefetch policy, 0|1 for always|tagged"<<endl;
	int prefetch_policy = 1;
	cout<<"Prefetch policy: tagged-prefetch"<<endl;

	// cout<<"Put in prefetch number, 0-4 (0 means no prefetch)"<<endl;
	int prefetch_num = 2;
	cout<<"Prefetch number: 2"<<endl;

	// cout<<"Put in bypass limit, 0-100"<<endl;
	int bypass_limit = 3;
	cout<<"Bypass limit: 3"<<endl;

	CacheConfig stat1(32*1024, 64, 8, 0, 1, replace_policy, prefetch_policy, prefetch_num, bypass_limit);
	CacheConfig stat2(256*1024, 512, 8, 0, 1, replace_policy, prefetch_policy, prefetch_num, bypass_limit);
	level1.SetConfig(stat1);
	level2.SetConfig(stat2);

	// Set the latency of memory
	StorageLatency tmpLatency;
	tmpLatency.bus_latency=0;
	tmpLatency.hit_latency=100;
	memory.SetLatency(tmpLatency);

	// Set the latency of level1
	tmpLatency.bus_latency=0;
	tmpLatency.hit_latency=3;
	level1.SetLatency(tmpLatency);

	// Set the latency of level2
	tmpLatency.bus_latency=6;
	tmpLatency.hit_latency=4;
	level2.SetLatency(tmpLatency);

	// Set the lower level of each cache
	level1.SetLower(&level2);
	level2.SetLower(&memory);

	// Open the file
	ifstream traceFile;


	// Some temporary variables
	char content[64];
	unsigned long long addr;
	char method;

	// These variables are used to count
	int count=0;
	int hit=0;
	int cycle=0;
    for (int i=0; i< 100; i++)
    {
        traceFile.open(argv[1]);
        while(traceFile>>method>>hex>>addr)
        {
            count++;
            if(method=='w')
                level1.HandleRequest(addr, 0, 0, content, hit, cycle);
            else
                level1.HandleRequest(addr, 0, 1, content, hit, cycle);
        }
        traceFile.close();
    }
    //AMAT= Hit Latency + Miss Rate * Miss Penalty
	// Print the result
	
	cout<<"Results:"<<endl;
	cout<<"Trace count: "<<count<<endl;
	cout<<"Hit count: "<<hit<<endl;
    cout << "Miss rate: " << 1-(double)hit/count << endl;
	cout<<"Time: "<<cycle<<endl;
	
	level1.PrintStats();
	level2.PrintStats();
	StorageStats l1_stats, l2_stats;
	StorageLatency l1_latency, l2_latency, mem_latency;
	level1.GetLatency(l1_latency);
	level2.GetLatency(l2_latency);
	memory.GetLatency(mem_latency);
	level1.GetStats(l1_stats);
	level2.GetStats(l2_stats);
	double AMAT = l1_latency.hit_latency + 
	l1_stats.miss_rate*(l2_latency.bus_latency + l2_latency.hit_latency
	+ l2_stats.miss_rate*(mem_latency.hit_latency));
    printf("AMAT: %lf\n", AMAT);

	return 0;
}
