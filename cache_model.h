// This is a simple c-based cache simulator which accumulates access time as it is queried
// by some externally generated addresses. It is a very simple direct mapped cache with variable
// cache line length, size and latencies. To simulate say, 100k reads to a random address, just
// initialize the model, set the cache mode (off or on), and then generate 100k calls to
// cm_do_access(<address>) choosing a new random address each time. You can get the accumulated
// time with the call (int) cm_get_last_access_cycles(), as often as you like. The result is
// cumulative.

#ifndef _CACHE_MODEL_H
#define _CACHE_MODEL_H

#define CM_LINE_SIZE 8
#define CM_CACHE_LINES 256 
#define CM_ADDRESS_SPACE_SIZE 65536
#define CM_DRAM_LATENCY 8
#define CM_DRAM_CYCLES_PER_WORD 1
#define CM_CACHE_UPDATE_CYCLES 3
#define CM_DRAM_BUFFER_SIZE 4


// Its neater if you keep the model and your code in two seperate files
// Compile your program with something like
//  gcc main.c cache_model.c -lm
//
// Usage: call cm_init() at the start of your program.
//
// At the start of each simulation run call either cm_enable_cache() or
// cm_disable_cache() to set-up, each of these automatically call
// cm_reset_model() to clear old state.
//
// Per simulated access, call cm_do_access() with the address you randomly 
// generated, then call cm_get_last_access_cycles() to query the simulated
// time that took.

extern void cm_init();
extern void cm_do_access(int address);
extern void cm_reset_model();
extern void cm_enable_cache();
extern void cm_disable_cache();
extern int cm_get_last_access_cycles();

// provides a random integer from 0 to b-1
extern int rand_int(int b);
// jumps the random generator by 2^96 -- provides a new, unrelated random sequence for rand_int()
// unlrelated sequence is limited to 2^32 calls due to state of 2^128
extern void rand_jump();

#endif 
