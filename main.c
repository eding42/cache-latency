
#include "cache_model.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// dump everything to a file.
#include <stdarg.h>
static void dump_samples(const char *fname, const int *samples, int n) {
    FILE *f = fopen(fname, "w");
    if (!f) { perror("fopen"); exit(1); }
    for (int i = 0; i < n; ++i) fprintf(f, "%d\n", samples[i]);
    fclose(f);
}


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

// make sure that we do not go out of bounds. 

static inline int wrap_addr(int x) {
    if (x >= CM_ADDRESS_SPACE_SIZE) x -= CM_ADDRESS_SPACE_SIZE;
    if (x < 0)                      x += CM_ADDRESS_SPACE_SIZE;
    return x;
}

static int addr_locality_next(int current, double s, double p) {
    int r = rand_int(10000); // random integer between 0 and 10000
    int s_th = (int)(s * 10000.0 + 0.5); 
    int p_th = s_th + (int)(p * 10000.0 + 0.5);

    if (r < s_th) {  
        return wrap_addr(current + 1); // 60% chance sequential access
    } else if (r < p_th) {
        
        int delta;
        do {
            delta = rand_int(81) - 40;  // [-40, +40]
        } while (delta == 0 || delta == 1); // keep spinning if it is sequential
        return wrap_addr(current + delta); // if not sequential, it's within 40 words, do memory address
    } else {
        return rand_int(CM_ADDRESS_SPACE_SIZE); // else, 5% of totally random. 
    }
}

static double simulate_mean_cycles_locality(int trials, int cache_on, double s, double p) {
    long long total = 0;

    if (cache_on) cm_enable_cache();
    else          cm_disable_cache();

    int addr = rand_int(CM_ADDRESS_SPACE_SIZE);

    for (int i = 0; i < trials; ++i) {
        addr = addr_locality_next(addr, s, p);
        cm_do_access(addr);
        int cyc = cm_get_last_access_cycles();
        total += cyc;
    }
    return (double) total / (double)trials;
}



// int main(void) {
//     cm_init();

//     const int trials = 100000;
//     const double s = 0.60;
//     const double p = 0.35;

//     /* (a) uniform random — optional to keep for comparison */
//     double mean_on_a  = simulate_mean_cycles(trials, 1);
//     double mean_off_a = simulate_mean_cycles(trials, 0);

//     /* (b) locality model */
//     double mean_on_b  = simulate_mean_cycles_locality(trials, 1, s, p);
//     double mean_off_b = simulate_mean_cycles_locality(trials, 0, s, p);

//     printf("Simulation over %d trials (address space = %d)\n", trials, CM_ADDRESS_SPACE_SIZE);

//     printf("\n(a) Uniform random:\n");
//     printf("  Cache ENABLED : %.6f cycles (mean)\n", mean_on_a);
//     printf("  Cache DISABLED: %.6f cycles (mean)\n", mean_off_a);

//     printf("\n(b) Locality model (s=%.2f sequential, p=%.2f near±40, far=%.2f):\n", s, p, 1.0 - s - p);
//     printf("  Cache ENABLED : %.6f cycles (mean)\n", mean_on_b);
//     printf("  Cache DISABLED: %.6f cycles (mean)\n", mean_off_b);

//     return 0;
// }

static int gen_uniform(int _) { (void)_; return rand_int(CM_ADDRESS_SPACE_SIZE); }


static int gen_locality_next(int cur){
    /* s=0.60, p=0.35, far=0.05 */
    int r = rand_int(10000);
    if (r < 6000) {                // sequential
        return wrap_addr(cur + 1);
    } else if (r < 9500) {         // near ±40, not 0 or +1
        int d; do { d = rand_int(81) - 40; } while (d == 0 || d == 1);
        return wrap_addr(cur + d);
    } else {                       // far
        return rand_int(CM_ADDRESS_SPACE_SIZE);
    }
}

/* Replace your simulate_mean_* with a version that optionally collects samples */
static double simulate_collect(int trials, int cache_on, int (*addr_gen)(int), int start_addr,
                               int *out_samples /* may be NULL */) {
    long long total = 0;
    if (cache_on) cm_enable_cache(); else cm_disable_cache();

    int addr = start_addr;
    for (int i = 0; i < trials; ++i) {
        addr = addr_gen(addr);
        cm_do_access(addr);
        int cyc = cm_get_last_access_cycles();
        total += cyc;
        if (out_samples) out_samples[i] = cyc;
    }
    return (double)total / (double)trials;
}


int main(void) {
    cm_init();
    const int trials = 100000;

    /* Allocate sample buffers (stack or malloc ok) */
    int *r_on  = malloc(trials * sizeof(int));
    int *r_off = malloc(trials * sizeof(int));
    int *l_on  = malloc(trials * sizeof(int));
    int *l_off = malloc(trials * sizeof(int));
    if (!r_on || !r_off || !l_on || !l_off) { perror("malloc"); return 1; }

    /* (a) uniform */
    double mean_on_a  = simulate_collect(trials, 1, gen_uniform, 0, r_on);
    double mean_off_a = simulate_collect(trials, 0, gen_uniform, 0, r_off);

    /* (b) locality */
    int start = gen_uniform(0);
    double mean_on_b  = simulate_collect(trials, 1, gen_locality_next, start, l_on);
    double mean_off_b = simulate_collect(trials, 0, gen_locality_next, start, l_off);

    /* Dump CSVs */
    dump_samples("random_on.csv",       r_on,  trials);
    dump_samples("random_off.csv",      r_off, trials);
    dump_samples("locality_on.csv",     l_on,  trials);
    dump_samples("locality_off.csv",    l_off, trials);

    printf("Means — (a) random: on=%.6f off=%.6f | (b) locality: on=%.6f off=%.6f\n",
           mean_on_a, mean_off_a, mean_on_b, mean_off_b);

    free(r_on); free(r_off); free(l_on); free(l_off);
    return 0;
}



