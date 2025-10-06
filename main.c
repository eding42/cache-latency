
#include "cache_model.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static double simulate_mean_cycles(int trials, int cache_on) {
    long long total = 0;

    if (cache_on) cm_enable_cache();
    else          cm_disable_cache();

    for (int i = 0; i < trials; ++i) {
        int addr = rand_int(CM_ADDRESS_SPACE_SIZE);
        cm_do_access(addr);
        total += (double) cm_get_last_access_cycles();
    }
    return (double) total / (double)trials;
}



int main(){
    cm_init();

    int trials = 1000000;

    double mean_on  = simulate_mean_cycles(trials, 1);
    double mean_off = simulate_mean_cycles(trials, 0);
    
    printf("Simulation (uniform random over %d addresses)\n", CM_ADDRESS_SPACE_SIZE);
    printf("  Trials per mode: %d\n", trials);
    printf("  Cache ENABLED : %.6f cycles (mean)\n", mean_on);
    printf("  Cache DISABLED: %.6f cycles (mean)\n", mean_off);



}