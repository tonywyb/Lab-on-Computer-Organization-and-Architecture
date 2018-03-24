#include "cache.h"
#include "def.h"
#include <string.h>
#include <limits.h>
#include <iostream>

using namespace std;

int log2(uint64_t x) {
    int res = -1;
    while(x) {
		res ++;
        x >>= 1;
    }
    return res;
}

/*
 * Cache: SetConfig
 */
void Cache::SetConfig(CacheConfig cc) {
    config_ = cc;
    access_count = 0;
    set = new Set[config_.set_num];
    for(int i=0; i<config_.set_num; i++) {
        set[i].line = new Line[config_.associativity];
        memset( set[i].line, 0, config_.associativity * sizeof(Line) );
        for(int j=0; j<config_.associativity; j++) {
            set[i].line[j].block = new unsigned char[ config_.size/config_.set_num/config_.associativity ];
        }
    }
}

/*
* This function is used to determine which line is about to be replaced.
* The line is maybe an empty line (the valid equals to 0),
* or its last_access is the least.
* The return value is the line number.
*/
int Cache::LRU_line(int set_index)
{
	int line_num = 0;
	int min_access = INT_MAX;
	for (int i = 0; i < config_.associativity; i++)
	{
		if (set[set_index].line[i].valid == 0)
			return i;
		if (min_access > set[set_index].line[i].last_access)
		{
			min_access = set[set_index].line[i].last_access;
			line_num = i;
		}
	}
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
//             3|4 for write|read in prefetch
// [i|o] content: in|out data
// [out] hit: 0|1 for miss|hit
// [out] cycle: total access cycle
void Cache::HandleRequest(uint64_t addr, int bytes, int read,
	char *content, int &hit, int &cycle) {

	access_count++;
	// Decide on whether a bypass should take place.
	if (!BypassDecision()) {
		// Generate some infomation for the data partition.
		PartitionAlgorithm();
		// Check whether the required data already exist.
		uint64_t line_index;
		int replace_decision = ReplaceDecision(addr, line_index);
		if (replace_decision) {
			// Hit!
			hit += 1;
			cycle += latency_.bus_latency + latency_.hit_latency;

			// According to S = 2^s
			int s = log2(config_.set_num);
			// According to B = 2^b
			int b = log2(config_.block_num);
			// According to addr = [tag, set_index, block_index]
			uint64_t tag = TAG(addr, s, b);
			uint64_t set_index = SET_INDEX(addr, s, b);
			uint64_t block_index = BLOCK_INDEX(addr, s, b);

			// Update last_access of current line
			set[set_index].line[line_index].last_access = access_count;

			// write
			if (read == 0) {
				memcpy(set[set_index].line[line_index].block + block_index, content, bytes);
				// write-back
				if (config_.write_through == 0) {
					set[set_index].line[line_index].dirty = 1;
				}
				// write-through
				if (config_.write_through == 1) {
					lower_->HandleRequest(addr, bytes, read, content, hit, cycle);
				}
			}
			// read
			if (read == 1) {
				memcpy(content, set[set_index].line[line_index].block + block_index, bytes);
			}

			return;
		}

		// Choose a victim for the current request.
		ReplaceAlgorithm(addr, bytes, read, content, hit, cycle);

		// Decide on whether a prefetch should take place.
		if (PrefetchDecision()) {
			// Fetch the other data via HandleRequest() recursively.
			// To distinguish from the normal requests,
			// the 2|3 is employed for prefetched write|read data
			// while the 0|1 for normal ones.
			PrefetchAlgorithm();
		}
	}
	// Fetch from the lower layer

	// If a victim is selected, replace it.
	// Something else should be done here
	// according to your replacement algorithm.
}

int Cache::BypassDecision() {
  return FALSE;
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
// [i|o] content: in|out data
// [out] hit: 0|1 for miss|hit
// [out] cycle: total access cycle
void Cache::ReplaceAlgorithm(uint64_t addr, int bytes, int read,
	char *content, int &hit, int &cycle) {

	// If the operation is write and the cache is write-without-allocate
	if ((read == 0) && config_.write_allocate == 0)
	{
		// Call next level to handle
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
	int replace_line_num = LRU_line(set_index);

	Line & replace_line = set[set_index].line[replace_line_num];

	if (replace_line.last_access != 0)				// If last_access doesn't equals to 0, it means we have to replace it
	{
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
			lower_->HandleRequest(new_addr, new_bytes, new_read, new_content, hit, cycle);
		}
	}

	uint64_t new_addr = addr - block_index;		
	int new_bytes = config_.block_num;				
	char *new_content = (char *)replace_line.block;	
	int new_read = 1;							

	lower_->HandleRequest(new_addr, new_bytes, new_read, new_content, hit, cycle);


	if (read == 0)		//write
	{
		replace_line.dirty = 1;		
		replace_line.valid = 1;
		replace_line.tag = tag;
		replace_line.last_access = access_count;
		// Modify the data in blocks
		for (int i = 0; i < bytes; i++)
			replace_line.block[i + block_index] = content[i];
	}
	else					//read
	{
		replace_line.dirty = 0;	
		replace_line.valid = 1;
		replace_line.tag = tag;
		replace_line.last_access = access_count;
		for (int i = 0; i < bytes; i++)
			content[i] = replace_line.block[block_index + i];
	}
}

int Cache::PrefetchDecision() {
  return FALSE;
}

void Cache::PrefetchAlgorithm() {
}
