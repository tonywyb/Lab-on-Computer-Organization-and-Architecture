#include "stdio.h"
#include "cache.h"
#include "memory.h"
#include <fstream>
#include <iostream>
#include <stdlib.h>

using namespace std;

int main(int argc, char *argv[])
{
	if (argc<2 || argc>2)
	{
		cout<<"please enter exactly two argument"<<endl;
		return 0;
	}
	// Allocate new caches and memory
	Cache level1, level2;
	Memory memory;

	// Set the basic state of memory
	StorageStats tmpStats;
	tmpStats.access_cycle=0;
	memory.SetStats(tmpStats);

	cout<<"Replace policy: ARC"<<endl;

	cout<<"If bypass? (0|1 for no|yes)"<<endl;
	int bypass_policy;
	cin >> bypass_policy;


	cout<<"l1: Prefetch 2 blocks for each miss"<<endl;
	cout<<"l2: Prefetch 4 blocks for each miss"<<endl;
	int l1_prefetch_num = 2;
	int l2_prefetch_num = 4;

	CacheConfig stat1(32*1024, 64, 8, 0, 1, l1_prefetch_num, bypass_policy);
	CacheConfig stat2(256*1024, 512, 8, 0, 1, l2_prefetch_num, bypass_policy);
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

	ifstream traceFile;
	char content[64];
	unsigned long long addr;
	char method;
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
	cout << "iteration time: 100" << endl; 
	// Print the result for each level
	level1.PrintStats();
	level2.PrintStats();

	//AMAT= Hit Latency + Miss Rate * Miss Penalty
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
