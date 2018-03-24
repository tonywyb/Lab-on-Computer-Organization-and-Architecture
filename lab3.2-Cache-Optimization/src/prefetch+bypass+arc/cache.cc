#include "cache.h"
#include "def.h"
#include <string.h>
#include <limits.h>
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <vector>
#include <assert.h>
using namespace std;

vector<vector<int> > lru;
vector<vector<int> > lfu;
vector<vector<uint64_t> > lru_gost;
vector<vector<uint64_t> > lfu_gost;
/*
 * Calculate n, where x = 2^n
 */
int log2(uint64_t x) {
    int n = -1;
    // Shift right to count
    while(x) {
        x >>= 1;
        n++;
    }
    return n;
}

/*
 * Cache: SetConfig
 */
void Cache::SetConfig(CacheConfig cc) {
    config_ = cc;
    // Initialize the access_count;
    access_count = 0;
	
	// Initialize random number
	srand(time(0));

    // Allocate for set
    set = new Set[config_.set_num];
    // Generate each set
    for(int i=0; i<config_.set_num; i++) {

        // Allocate for line
        set[i].line = new Line[config_.associativity];
        set[i].lru_num = config_.associativity/2;
        set[i].lfu_num = config_.associativity/2;
        set[i].lfu_gost_num = config_.associativity/2;
        set[i].lru_gost_num = config_.associativity/2;
        memset(set[i].line, 0, config_.associativity * sizeof(Line));
        // Generate each line
        for(int j=0; j<config_.associativity; j++) {
            // Allocate for block
            set[i].line[j].block = new unsigned char[ config_.size/config_.set_num/config_.associativity ];
        }
	    lru.resize(config_.set_num);
	    lfu.resize(config_.set_num);
	    lru_gost.resize(config_.set_num);
	    lfu_gost.resize(config_.set_num);
    }

    // set stats_
    stats_.access_counter = 0;
    stats_.miss_num = 0;
    stats_.access_cycle = 0; 
    stats_.replace_num = 0; 
    stats_.fetch_num = 0; // Fetch lower layer
    stats_.prefetch_num = 0; 
}

int Cache::LRU_line(int set_index, int& if_free_line)
{
	int line_num = 0;
	for (int i = 0; i < config_.associativity; i++)
	{
		if (set[set_index].line[i].valid == 0)
		{
			if_free_line = 1;
			return i;
		}
	}
	line_num = lru[set_index].back();
	if_free_line = 0;
	return line_num;
}
/*
 * Cache: GetConfig
 */
void Cache::GetConfig(CacheConfig &cc) {
    cc = config_;
}

// Main access process
// [in]  addr: access address
// [in]  bytes: target number of bytes
// [in]  read: 0|1 for write|read
//             2|3 for write|read in prefetch
// [i|o] content: in|out data
// [out] hit: 0|1 for miss|hit
// [out] cycle: total access cycle
void Cache::HandleRequest(uint64_t addr, int bytes, int read,
	char *content, int &hit, int &cycle) {

	// According to S = 2^s
	int s = log2(config_.set_num);
	// According to B = 2^b
	int b = log2(config_.block_num);
	// According to addr = [tag, set_index, block_index]
	uint64_t tag = TAG(addr, s, b);
	uint64_t set_index = SET_INDEX(addr, s, b);
	uint64_t block_index = BLOCK_INDEX(addr, s, b);

	// increase access_count
	if(read!=3 && read != 2) {
		access_count++;
		stats_.access_counter++;
	}
	
	// Decide on whether a bypass should take place.
	if (!BypassDecision()) {
		// Generate some infomation for the data partition.
		PartitionAlgorithm();
		
		// Add latency
		if(read!=3 && read != 2) {
			cycle += latency_.bus_latency + latency_.hit_latency;
			stats_.access_cycle += latency_.bus_latency + latency_.hit_latency;			
		}
			
		// Check whether the required data already exist.
		uint64_t line_index;
		int replace_decision = ReplaceDecision(addr, line_index);
		if (replace_decision) {
			if(read != 3 && read != 2) {
				// Hit!
				hit += 1;
				// Update last_access of current line
				set[set_index].line[line_index].last_access = access_count;
				set[set_index].line[line_index].access_count++;
			}
			int if_in_lru = 0;
			//hit in lru_list
			for(int i=0;i<lru[set_index].size();++i)
			{
				if(lru[set_index][i] == line_index)
				{
					if (lru[set_index].size() > 1)
					{
						lru[set_index].erase(lru[set_index].begin() + i);
					}
					if_in_lru = 1;
					if(lfu[set_index].size() < set[set_index].lfu_num)
					{
						lfu[set_index].insert(lfu[set_index].begin(), line_index);
					}
					else
					{
						int tmp = lfu[set_index].back();
						lfu[set_index].pop_back();
						lfu[set_index].insert(lfu[set_index].begin(), line_index);
						if(lfu_gost[set_index].size() < set[set_index].lfu_gost_num)
						{
							lfu_gost[set_index].insert(lfu_gost[set_index].begin(), set[set_index].line[tmp].tag);
						}
						else
						{
							lfu_gost[set_index].pop_back();
							lfu_gost[set_index].insert(lfu_gost[set_index].begin(), set[set_index].line[tmp].tag);
						}
					}
					break;
				}
			}
			//hit in lfu_list
			if (if_in_lru == 0)
			{
				for(int i=0;i<lfu[set_index].size();++i)
				{
					if(lfu[set_index][i] == line_index)
					{
						lfu[set_index].erase(lfu[set_index].begin()+i);
						lfu[set_index].insert(lfu[set_index].begin(), line_index);
					}
				}		
			}
			// write
			if (read == 0 || read == 2) {
				memcpy(set[set_index].line[line_index].block + block_index, content, bytes);
				// write-back
				if (config_.write_through == 0) {
					set[set_index].line[line_index].dirty = 1;
				}
				// write-through
				if (config_.write_through == 1) {
					stats_.fetch_num++;
					lower_->HandleRequest(addr, bytes, read, content, hit, cycle);
				}
			}
			// read
			if (read == 1 || read == 3) {
				memcpy(content, set[set_index].line[line_index].block + block_index, bytes);
			}
		}
		else {
			// Miss: choose a victim for the current request.
			if(read!=3 && read != 2) {
				stats_.miss_num++;
				stats_.prefetch_num++;
				PrefetchAlgorithm(addr, replace_decision);
			}
			ReplaceAlgorithm(addr, bytes, read, content, hit, cycle);
		}	
		return;
	}
	
	// Fetch from the lower layer
	// Add latency
	if(read != 3 && read != 2) {
		cycle += latency_.bus_latency;
		stats_.access_cycle += latency_.bus_latency;
	}
	lower_->HandleRequest(addr, bytes, read, content, hit, cycle);
}

int Cache::BypassDecision() {
	if (config_.bypass_policy == 0)
	{
		return false;
	}
	if (config_.set_num == 64) // l1 never bypass
	{
		return false;
	}
	else if (config_.set_num == 512) //l2 bypass when miss_rate is higher than 0.9
	{
		if (stats_.access_counter <= 100)
		{
			return false;
		}
		else
		{
			if (double(stats_.miss_num)/double(stats_.access_counter) > 0.8)
			{
				return true;
			}
		}
	}
	return false;
}

void Cache::PartitionAlgorithm() {
}

/*
 * Cache: ReplaceDecision
 */
int Cache::ReplaceDecision(uint64_t addr, uint64_t &line_index) {
    // According to S = 2^s
    int s = log2(config_.set_num);
    // According to B = 2^b
    int b = log2(config_.block_num);
    // According to addr = [tag, set_index, block_index]
    uint64_t tag = TAG(addr, s, b);
    uint64_t set_index = SET_INDEX(addr, s, b);
    uint64_t block_index = BLOCK_INDEX(addr, s, b);

    // look for given tag
    for(int i=0; i<config_.associativity; i++) {
        if(set[set_index].line[i].tag == tag && set[set_index].line[i].valid == 1) {
            line_index = i;
            return TRUE;
        }
    }

    return FALSE;
}

// [in]  addr: access address
// [in]  bytes: target number of bytes
// [in]  read: 0|1 for write|read
//             2|3 for write|read in prefetch
// [i|o] content: in|out data
// [out] hit: 0|1 for miss|hit
// [out] cycle: total access cycle
void Cache::ReplaceAlgorithm(uint64_t addr, int bytes, int read,
	char *content, int &hit, int &cycle) {

	// If the operation is write and the cache is write-without-allocate
	if (read == 0 && config_.write_allocate == 0)
	{
        // Call next level to handle
		stats_.fetch_num++;
		lower_->HandleRequest(addr, bytes, read, content, hit, cycle);
		// Return immediately
		return;
	}
	// According to S = 2^s
	uint64_t s = log2(config_.set_num);
	// According to B = 2^b
	uint64_t b = log2(config_.block_num);
	// According to addr = [tag, set_index, block_index]
	uint64_t tag = TAG(addr, s, b);
	uint64_t set_index = SET_INDEX(addr, s, b);
	uint64_t block_index = BLOCK_INDEX(addr, s, b);

	// replace_line_num stores line number of the replace-line
	int if_free = -1;
	int if_in_lru_gost = 0;
	int if_in_lfu_gost = 0;
	for (int i = 0;i < lru_gost[set_index].size();++i)
	{
		if(lru_gost[set_index][i] == tag)
		{
			if_in_lru_gost = 1;
			lru_gost[set_index].erase(lru_gost[set_index].begin()+i);
			break;
		}
	}
	for (int i = 0;i < lfu_gost[set_index].size();++i)
	{
		if(lfu_gost[set_index][i] == tag)
		{
			if_in_lfu_gost = 1;
			lfu_gost[set_index].erase(lfu_gost[set_index].begin()+i);
			break;
		}
	}
	if (if_in_lru_gost == 1 && set[set_index].lfu_num > 0)
	{
		set[set_index].lru_num++;
		set[set_index].lfu_num--;
		assert(set[set_index].lru_num + set[set_index].lfu_num == config_.associativity);
	}
	if (if_in_lfu_gost == 1 && set[set_index].lru_num > 1)
	{
		set[set_index].lfu_num++;
		set[set_index].lru_num--;
		assert(set[set_index].lru_num + set[set_index].lfu_num == config_.associativity);
	}
	int replace_line_num = LRU_line(set_index, if_free); //if_free=0|1:exist line|free line
	if (if_free == 1 && lru[set_index].size() < set[set_index].lru_num)// convict a free line
	{
		lru[set_index].insert(lru[set_index].begin(), replace_line_num);
	}
	else 
	{
		if (lru[set_index].size() != 0)
		{
			if (if_free == 1)
			{
				replace_line_num = lru[set_index].back();
			}
			lru[set_index].insert(lru[set_index].begin(), replace_line_num);
		}
	}
	int delta = lru[set_index].size() - set[set_index].lru_num;
	for(int i = lru[set_index].size() - 1; delta > 0; --i, --delta)
	{
		lru_gost[set_index].insert(lru_gost[set_index].begin(), lru[set_index][i]);
		lru[set_index].pop_back();
	}
	for(delta = lru_gost[set_index].size() - set[set_index].lru_gost_num;delta > 0;--delta)
	{
		lru_gost[set_index].pop_back();
	}
	delta = lfu[set_index].size() - set[set_index].lfu_num;
	for(int i = lfu[set_index].size() - 1; delta > 0; --i, --delta)
	{
		lfu_gost[set_index].insert(lfu_gost[set_index].begin(), lfu[set_index][i]);
		lfu[set_index].pop_back();
	}
	for(delta = lfu_gost[set_index].size() - set[set_index].lfu_gost_num; delta > 0;--delta)
	{
		lfu_gost[set_index].pop_back();
	}
	
	Line & replace_line = set[set_index].line[replace_line_num];
	if(read==3)
		replace_line.prefetch_tag = 1;
	if (replace_line.valid != 0)				// If last_access doesn't equals to 0, it means we have to replace it
	{
		stats_.replace_num++;
		// If the replace_line is dirty, we have to write the line back to next level
		if (replace_line.dirty == 1)
		{
			// Calculate the new_addr
			uint64_t new_addr = (replace_line.tag << (s + b)) + (set_index << b);
			// Calculate the new_bytes (equals to the block_num)
			int new_bytes = config_.block_num;
			// Copy the block content to new_content
			char *new_content = (char *)replace_line.block;
			// new_read is set to 0, as we wanna modify the data
			int new_read = 0;
			int new_hit = 0;

			// Write the old data to next level
			stats_.fetch_num++;
			if (read == 3)
			{
				new_read = 2;
			}
			lower_->HandleRequest(new_addr, new_bytes, new_read, new_content, new_hit, cycle);
		}
	}

	// Call next level to handle the request
	uint64_t new_addr = addr - block_index;		// new_addr is get by abstracting block offset
	int new_bytes = config_.block_num;					// new_bytes equals to the value of B
	char *new_content = (char *)replace_line.block;	// The space of new_content is determined by B
	int new_read = 1;							// new_read is set to 1 (means read), as we want to read data from next level

	// Other arguments remain the same
    stats_.fetch_num++;
	lower_->HandleRequest(new_addr, new_bytes, new_read, new_content, hit, cycle);

	// Set the basic information of the new line
	replace_line.valid = 1;
	replace_line.tag = tag;
	replace_line.last_access = access_count;
	replace_line.access_count = 1;
	replace_line.first_access = access_count;

	if(read == 3)
		replace_line.prefetch_tag = 1;
	
	if (read == 0 || read == 2)		// If it's write (specifically, write-with-allocate)
	{
		replace_line.dirty = 1;		// dirty-bit has to be set to 1
		// Modify the data in blocks
		for (int i = 0; i < bytes; i++)
			replace_line.block[i + block_index] = content[i];
	}
	else					// If it's read
	{
		replace_line.dirty = 0;	
		// Copy the data to content
		for (int i = 0; i < bytes; i++)
			content[i] = replace_line.block[block_index + i];
	}
}

int Cache::PrefetchDecision(uint64_t addr, int hit) {
	if(hit==0)	// If it's a miss
		return true;
	else
		return false;
}

void Cache::PrefetchAlgorithm(uint64_t addr, int hit) {
	// Prefetch shouldn't affect the hit and cycle count
	char tmp_content[64];
	int tmp_hit=0;
	int tmp_cycle=0;
	for(int i=1; i<config_.prefetch_num; i++)
		HandleRequest(addr+config_.block_num*i, 0, 3, tmp_content, tmp_hit, tmp_cycle);
}

