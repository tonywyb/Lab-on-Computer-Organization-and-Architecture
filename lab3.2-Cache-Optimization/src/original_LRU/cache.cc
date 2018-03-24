#include "cache.h"
#include "def.h"
#include <string.h>
#include <limits.h>
#include <iostream>
#include <cstdlib>
#include <time.h>

using namespace std;

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
        memset( set[i].line, 0, config_.associativity * sizeof(Line) );
        // Generate each line
        for(int j=0; j<config_.associativity; j++) {

            // Allocate for block
            set[i].line[j].block = new unsigned char[ config_.size/config_.set_num/config_.associativity ];
        }
    }

    // set stats_
    stats_.access_counter = 0;
    stats_.miss_num = 0;
    stats_.access_cycle = 0; // In nanoseconds
    stats_.replace_num = 0; // Evict old lines
    stats_.fetch_num = 0; // Fetch lower layer
    stats_.prefetch_num = 0; // Prefetch
}

/*
 * LRU: This function is used to determine which line is about to be replaced.
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

	// According to S = 2^s
	int s = log2(config_.set_num);
	// According to B = 2^b
	int b = log2(config_.block_num);
	// According to addr = [tag, set_index, block_index]
	uint64_t tag = TAG(addr, s, b);
	uint64_t set_index = SET_INDEX(addr, s, b);
	uint64_t block_index = BLOCK_INDEX(addr, s, b);

	// increase access_count
	//if(read!=3) {
		access_count++;
		stats_.access_counter++;
	//}

	/*  Vital Branch!!!  */
	/*
	int overblock = bytes+block_index-config_.block_num;
	if(overblock>0) {
		HandleRequest(addr, bytes-overblock, read, content, hit, cycle);
		HandleRequest(addr+bytes-overblock, overblock, read, content+bytes-overblock, hit, cycle);
		return;
	}
	*/

	// Decide on whether a bypass should take place.
	if (!BypassDecision()) {
		// Generate some infomation for the data partition.
		PartitionAlgorithm();

		// Add latency
		if(read!=3) {
			cycle += latency_.bus_latency + latency_.hit_latency;
			stats_.access_cycle += latency_.bus_latency + latency_.hit_latency;
		}

		// Check whether the required data already exist.
		uint64_t line_index;
		int replace_decision = ReplaceDecision(addr, line_index);
		if (replace_decision) {
			if(read!=3) {
				// Hit!
				hit += 1;

				// Update last_access of current line
				set[set_index].line[line_index].last_access = access_count;
				set[set_index].line[line_index].access_count++;
			}

			// write
			if (read == 0) {
				// memcpy(set[set_index].line[line_index].block + block_index, content, bytes);
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
			if (read == 1) {
				memcpy(content, set[set_index].line[line_index].block + block_index, bytes);
			}
		}
		else {

			// Miss: choose a victim for the current request.
			if(read!=3) {
				stats_.miss_num++;
			}
			ReplaceAlgorithm(addr, bytes, read, content, hit, cycle);
		}

		// Decide on whether a prefetch should take place.
		if (read!=3&&PrefetchDecision(addr, replace_decision)) {
			// Fetch the other data via HandleRequest() recursively.
			// To distinguish from the normal requests,
			// the 2|3 is employed for prefetched write|read data
			// while the 0|1 for normal ones.
			stats_.prefetch_num++;
			PrefetchAlgorithm(addr, replace_decision);
		}

		return;
	}

	// Fetch from the lower layer
	// Add latency
	if(read!=3) {
		cycle += latency_.bus_latency;
		stats_.access_cycle += latency_.bus_latency;
	}
	lower_->HandleRequest(addr, bytes, read, content, hit, cycle);

}

int Cache::BypassDecision() {

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
	int replace_line_num = LRU_line(set_index);

	Line & replace_line = set[set_index].line[replace_line_num];
	// whether the old line is valid
	if(replace_line.valid) {
		stats_.replace_num++;
	}

	if (replace_line.valid != 0)				// If last_access doesn't equals to 0, it means we have to replace it
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
			int new_hit = 0;

			// Write the old data to next level
            stats_.fetch_num++;
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

	if (read == 0)		// If it's write (specifically, write-with-allocate)
	{
		replace_line.dirty = 1;		// dirty-bit has to be set to 1

		// Modify the data in blocks
		for (int i = 0; i < bytes; i++)
			replace_line.block[i + block_index] = content[i];
	}
	else					// If it's read
	{
		replace_line.dirty = 0;		// dirty-bit is set to 0

		// Copy the data to content
		for (int i = 0; i < bytes; i++)
			content[i] = replace_line.block[block_index + i];
	}
}

int Cache::PrefetchDecision(uint64_t addr, int hit) {
	return false;
}

void Cache::PrefetchAlgorithm(uint64_t addr, int hit) {
	return;
}

