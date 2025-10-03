#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ISA-L functions
extern void ec_init_tables(int k, int rows, unsigned char *a, unsigned char *g_tbls);
extern void gf_vect_dot_prod_base(int len, int vlen, unsigned char *v, unsigned char **src, unsigned char *dest);

// NEON functions
extern void gf_vect_dot_prod_neon(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char *dest);
extern void gf_2vect_dot_prod_neon(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);
extern void gf_3vect_dot_prod_neon(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);
extern void gf_4vect_dot_prod_neon(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);
extern void gf_5vect_dot_prod_neon(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);

// SVE Assembly functions
extern void gf_vect_dot_prod_sve(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char *dest);
extern void gf_2vect_dot_prod_sve(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);
extern void gf_3vect_dot_prod_sve(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);
extern void gf_4vect_dot_prod_sve(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);
extern void gf_5vect_dot_prod_sve(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);

// SVE Intrinsics (original) functions
extern void gf_vect_dot_prod_sve_intrinsics(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char *dest);
extern void gf_2vect_dot_prod_sve_intrinsics(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);
extern void gf_3vect_dot_prod_sve_intrinsics(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);
extern void gf_4vect_dot_prod_sve_intrinsics(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);
extern void gf_5vect_dot_prod_sve_intrinsics(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);

// SVE V2 (generic) functions
extern void gf_1vect_dot_prod_sve_intrinsics_v2(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char *dest);
extern void gf_2vect_dot_prod_sve_intrinsics_v2(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);
extern void gf_3vect_dot_prod_sve_intrinsics_v2(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);
extern void gf_4vect_dot_prod_sve_intrinsics_v2(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);
extern void gf_5vect_dot_prod_sve_intrinsics_v2(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);
extern void gf_6vect_dot_prod_sve_intrinsics_v2(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);
extern void gf_7vect_dot_prod_sve_intrinsics_v2(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);

// SVE V3 (simplified) functions
extern void gf_1vect_dot_prod_sve_intrinsics_v3(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char *dest);
extern void gf_2vect_dot_prod_sve_intrinsics_v3(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);
extern void gf_3vect_dot_prod_sve_intrinsics_v3(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);
extern void gf_4vect_dot_prod_sve_intrinsics_v3(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);
extern void gf_5vect_dot_prod_sve_intrinsics_v3(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);
extern void gf_6vect_dot_prod_sve_intrinsics_v3(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);
extern void gf_7vect_dot_prod_sve_intrinsics_v3(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

void run_base_test(const char *name, int nvect, int len, int k, unsigned char *g_tbls, 
                   unsigned char **src, unsigned char **dest, int iterations) {
    
    for (int d = 0; d < nvect; d++) {
        gf_vect_dot_prod_base(len, k, &g_tbls[d * k * 32], src, dest[d]);
    }
    
    double start = get_time();
    for (int iter = 0; iter < iterations; iter++) {
        for (int d = 0; d < nvect; d++) {
            gf_vect_dot_prod_base(len, k, &g_tbls[d * k * 32], src, dest[d]);
        }
        __asm__ volatile("" : : "r" (dest[0][0]) : "memory");
    }
    double elapsed = get_time() - start;
    
    double bytes_processed = (double)len * k * nvect * iterations;
    printf("%-25s: %.3f seconds (%.1f MB/s)\n", name, elapsed, bytes_processed / elapsed / 1e6);
}

void run_1vect_test(const char *name, void (*func)(int, int, unsigned char*, unsigned char**, unsigned char*),
                    int len, int k, unsigned char *g_tbls, unsigned char **src, unsigned char **dest, int iterations) {
    
    func(len, k, g_tbls, src, dest[0]);
    
    double start = get_time();
    for (int iter = 0; iter < iterations; iter++) {
        func(len, k, g_tbls, src, dest[0]);
        __asm__ volatile("" : : "r" (dest[0][0]) : "memory");
    }
    double elapsed = get_time() - start;
    
    double bytes_processed = (double)len * k * 1 * iterations;
    printf("%-25s: %.3f seconds (%.1f MB/s)\n", name, elapsed, bytes_processed / elapsed / 1e6);
}

void run_neon_6vect(int len, int k, unsigned char *g_tbls, unsigned char **src, unsigned char **dest, int iterations) {
    // NEON 6-vector: 5 + 1 combination (like ec_encode_data_neon)
    gf_5vect_dot_prod_neon(len, k, g_tbls, src, dest);
    gf_vect_dot_prod_neon(len, k, &g_tbls[5 * k * 32], src, dest[5]);
    
    double start = get_time();
    for (int iter = 0; iter < iterations; iter++) {
        gf_5vect_dot_prod_neon(len, k, g_tbls, src, dest);
        gf_vect_dot_prod_neon(len, k, &g_tbls[5 * k * 32], src, dest[5]);
        __asm__ volatile("" : : "r" (dest[0][0]) : "memory");
    }
    double elapsed = get_time() - start;
    
    double bytes_processed = (double)len * k * 6 * iterations;
    printf("%-25s: %.3f seconds (%.1f MB/s)\n", "NEON (5+1)", elapsed, bytes_processed / elapsed / 1e6);
}

void run_neon_7vect(int len, int k, unsigned char *g_tbls, unsigned char **src, unsigned char **dest, int iterations) {
    // NEON 7-vector: 5 + 2 combination (like ec_encode_data_neon)
    gf_5vect_dot_prod_neon(len, k, g_tbls, src, dest);
    gf_2vect_dot_prod_neon(len, k, &g_tbls[5 * k * 32], src, &dest[5]);
    
    double start = get_time();
    for (int iter = 0; iter < iterations; iter++) {
        gf_5vect_dot_prod_neon(len, k, g_tbls, src, dest);
        gf_2vect_dot_prod_neon(len, k, &g_tbls[5 * k * 32], src, &dest[5]);
        __asm__ volatile("" : : "r" (dest[0][0]) : "memory");
    }
    double elapsed = get_time() - start;
    
    double bytes_processed = (double)len * k * 7 * iterations;
    printf("%-25s: %.3f seconds (%.1f MB/s)\n", "NEON (5+2)", elapsed, bytes_processed / elapsed / 1e6);
}

void run_nvect_test(const char *name, void (*func)(int, int, unsigned char*, unsigned char**, unsigned char**),
                    int nvect, int len, int k, unsigned char *g_tbls, unsigned char **src, unsigned char **dest, int iterations) {
    
    func(len, k, g_tbls, src, dest);
    
    double start = get_time();
    for (int iter = 0; iter < iterations; iter++) {
        func(len, k, g_tbls, src, dest);
        __asm__ volatile("" : : "r" (dest[0][0]) : "memory");
    }
    double elapsed = get_time() - start;
    
    double bytes_processed = (double)len * k * nvect * iterations;
    printf("%-25s: %.3f seconds (%.1f MB/s)\n", name, elapsed, bytes_processed / elapsed / 1e6);
}

int verify_results(int nvect, int len, unsigned char **dest_base, unsigned char **dest_test) {
    int errors = 0;
    for (int d = 0; d < nvect && errors < 5; d++) {
        for (int i = 0; i < len && errors < 5; i++) {
            if (dest_base[d][i] != dest_test[d][i]) {
                errors++;
            }
        }
    }
    return errors;
}

void benchmark_nvect(int nvect) {
    const int len = 8192;
    const int k = 6;
    const int iterations = 2000;
    
    printf("\n%d-Vector Dot Product Benchmark\n", nvect);
    printf("==============================\n");
    printf("Data size: %d bytes, Source vectors: %d, Iterations: %d\n\n", len, k, iterations);
    
    // Allocate memory
    unsigned char *src[k];
    unsigned char *dest_base[nvect];
    unsigned char *dest_neon[nvect];
    unsigned char *dest_sve_asm[nvect];
    unsigned char *dest_sve_intrinsics[nvect];
    unsigned char *dest_sve_v2[nvect];
    unsigned char *dest_sve_v3[nvect];
    unsigned char *encode_matrix;
    unsigned char *g_tbls;
    
    srand(42 + nvect);
    for (int i = 0; i < k; i++) {
        src[i] = malloc(len);
        for (int j = 0; j < len; j++) {
            src[i][j] = rand() & 0xFF;
        }
    }
    
    for (int i = 0; i < nvect; i++) {
        dest_base[i] = malloc(len);
        dest_neon[i] = malloc(len);
        dest_sve_asm[i] = malloc(len);
        dest_sve_intrinsics[i] = malloc(len);
        dest_sve_v2[i] = malloc(len);
        dest_sve_v3[i] = malloc(len);
    }
    
    encode_matrix = malloc(nvect * k);
    for (int i = 0; i < nvect * k; i++) {
        encode_matrix[i] = (rand() & 0xFE) | 1;
    }
    
    g_tbls = malloc(nvect * k * 32);
    ec_init_tables(k, nvect, encode_matrix, g_tbls);
    
    // Generate reference results
    for (int d = 0; d < nvect; d++) {
        gf_vect_dot_prod_base(len, k, &g_tbls[d * k * 32], src, dest_base[d]);
    }
    
    // Test and benchmark each implementation
    printf("Performance Results:\n");
    printf("--------------------\n");
    
    run_base_test("Base (scalar)", nvect, len, k, g_tbls, src, dest_base, iterations);
    
    // Test available implementations based on nvect
    switch (nvect) {
        case 1:
            run_1vect_test("NEON", gf_vect_dot_prod_neon, len, k, g_tbls, src, dest_neon, iterations);
            run_1vect_test("SVE Assembly", gf_vect_dot_prod_sve, len, k, g_tbls, src, dest_sve_asm, iterations);
            run_1vect_test("SVE Intrinsics", gf_vect_dot_prod_sve_intrinsics, len, k, g_tbls, src, dest_sve_intrinsics, iterations);
            run_1vect_test("SVE V2", gf_1vect_dot_prod_sve_intrinsics_v2, len, k, g_tbls, src, dest_sve_v2, iterations);
            run_1vect_test("SVE V3", gf_1vect_dot_prod_sve_intrinsics_v3, len, k, g_tbls, src, dest_sve_v3, iterations);
            
            // Verify correctness
            if (verify_results(1, len, dest_base, dest_neon) == 0) printf("NEON: PASS\n");
            if (verify_results(1, len, dest_base, dest_sve_v3) == 0) printf("SVE V3: PASS\n");
            break;
            
        case 2:
            run_nvect_test("NEON", gf_2vect_dot_prod_neon, nvect, len, k, g_tbls, src, dest_neon, iterations);
            run_nvect_test("SVE Assembly", gf_2vect_dot_prod_sve, nvect, len, k, g_tbls, src, dest_sve_asm, iterations);
            run_nvect_test("SVE Intrinsics", gf_2vect_dot_prod_sve_intrinsics, nvect, len, k, g_tbls, src, dest_sve_intrinsics, iterations);
            run_nvect_test("SVE V2", gf_2vect_dot_prod_sve_intrinsics_v2, nvect, len, k, g_tbls, src, dest_sve_v2, iterations);
            run_nvect_test("SVE V3", gf_2vect_dot_prod_sve_intrinsics_v3, nvect, len, k, g_tbls, src, dest_sve_v3, iterations);
            
            if (verify_results(nvect, len, dest_base, dest_neon) == 0) printf("NEON: PASS\n");
            if (verify_results(nvect, len, dest_base, dest_sve_v3) == 0) printf("SVE V3: PASS\n");
            break;
            
        case 3:
            run_nvect_test("NEON", gf_3vect_dot_prod_neon, nvect, len, k, g_tbls, src, dest_neon, iterations);
            run_nvect_test("SVE Assembly", gf_3vect_dot_prod_sve, nvect, len, k, g_tbls, src, dest_sve_asm, iterations);
            run_nvect_test("SVE Intrinsics", gf_3vect_dot_prod_sve_intrinsics, nvect, len, k, g_tbls, src, dest_sve_intrinsics, iterations);
            run_nvect_test("SVE V2", gf_3vect_dot_prod_sve_intrinsics_v2, nvect, len, k, g_tbls, src, dest_sve_v2, iterations);
            run_nvect_test("SVE V3", gf_3vect_dot_prod_sve_intrinsics_v3, nvect, len, k, g_tbls, src, dest_sve_v3, iterations);
            
            if (verify_results(nvect, len, dest_base, dest_neon) == 0) printf("NEON: PASS\n");
            if (verify_results(nvect, len, dest_base, dest_sve_v3) == 0) printf("SVE V3: PASS\n");
            break;
            
        case 4:
            run_nvect_test("NEON", gf_4vect_dot_prod_neon, nvect, len, k, g_tbls, src, dest_neon, iterations);
            run_nvect_test("SVE Assembly", gf_4vect_dot_prod_sve, nvect, len, k, g_tbls, src, dest_sve_asm, iterations);
            run_nvect_test("SVE Intrinsics", gf_4vect_dot_prod_sve_intrinsics, nvect, len, k, g_tbls, src, dest_sve_intrinsics, iterations);
            run_nvect_test("SVE V2", gf_4vect_dot_prod_sve_intrinsics_v2, nvect, len, k, g_tbls, src, dest_sve_v2, iterations);
            run_nvect_test("SVE V3", gf_4vect_dot_prod_sve_intrinsics_v3, nvect, len, k, g_tbls, src, dest_sve_v3, iterations);
            
            if (verify_results(nvect, len, dest_base, dest_neon) == 0) printf("NEON: PASS\n");
            if (verify_results(nvect, len, dest_base, dest_sve_v3) == 0) printf("SVE V3: PASS\n");
            break;
            
        case 5:
            run_nvect_test("NEON", gf_5vect_dot_prod_neon, nvect, len, k, g_tbls, src, dest_neon, iterations);
            run_nvect_test("SVE Assembly", gf_5vect_dot_prod_sve, nvect, len, k, g_tbls, src, dest_sve_asm, iterations);
            run_nvect_test("SVE Intrinsics", gf_5vect_dot_prod_sve_intrinsics, nvect, len, k, g_tbls, src, dest_sve_intrinsics, iterations);
            run_nvect_test("SVE V2", gf_5vect_dot_prod_sve_intrinsics_v2, nvect, len, k, g_tbls, src, dest_sve_v2, iterations);
            run_nvect_test("SVE V3", gf_5vect_dot_prod_sve_intrinsics_v3, nvect, len, k, g_tbls, src, dest_sve_v3, iterations);
            
            if (verify_results(nvect, len, dest_base, dest_neon) == 0) printf("NEON: PASS\n");
            if (verify_results(nvect, len, dest_base, dest_sve_v3) == 0) printf("SVE V3: PASS\n");
            break;
            
        case 6:
            run_neon_6vect(len, k, g_tbls, src, dest_neon, iterations);
            run_nvect_test("SVE V2", gf_6vect_dot_prod_sve_intrinsics_v2, nvect, len, k, g_tbls, src, dest_sve_v2, iterations);
            run_nvect_test("SVE V3", gf_6vect_dot_prod_sve_intrinsics_v3, nvect, len, k, g_tbls, src, dest_sve_v3, iterations);
            
            if (verify_results(nvect, len, dest_base, dest_neon) == 0) printf("NEON: PASS\n");
            if (verify_results(nvect, len, dest_base, dest_sve_v3) == 0) printf("SVE V3: PASS\n");
            break;
            
        case 7:
            run_neon_7vect(len, k, g_tbls, src, dest_neon, iterations);
            run_nvect_test("SVE V2", gf_7vect_dot_prod_sve_intrinsics_v2, nvect, len, k, g_tbls, src, dest_sve_v2, iterations);
            run_nvect_test("SVE V3", gf_7vect_dot_prod_sve_intrinsics_v3, nvect, len, k, g_tbls, src, dest_sve_v3, iterations);
            
            if (verify_results(nvect, len, dest_base, dest_neon) == 0) printf("NEON: PASS\n");
            if (verify_results(nvect, len, dest_base, dest_sve_v3) == 0) printf("SVE V3: PASS\n");
            break;
    }
    
    // Cleanup
    for (int i = 0; i < k; i++) free(src[i]);
    for (int i = 0; i < nvect; i++) {
        free(dest_base[i]);
        free(dest_neon[i]);
        free(dest_sve_asm[i]);
        free(dest_sve_intrinsics[i]);
        free(dest_sve_v2[i]);
        free(dest_sve_v3[i]);
    }
    free(encode_matrix);
    free(g_tbls);
}

int main(int argc, char *argv[]) {
    printf("ISA-L N-Vector Dot Product Comprehensive Benchmark\n");
    printf("==================================================\n");
    
    if (argc > 1) {
        int nvect = atoi(argv[1]);
        if (nvect >= 1 && nvect <= 7) {
            benchmark_nvect(nvect);
            return 0;
        }
    }
    
    // Run all benchmarks
    for (int nvect = 1; nvect <= 7; nvect++) {
        benchmark_nvect(nvect);
    }
    
    return 0;
}
