#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Include ISA-L functions
extern void ec_init_tables(int k, int rows, unsigned char *a, unsigned char *g_tbls);

// Base implementation
extern void gf_vect_dot_prod_base(int len, int vlen, unsigned char *v, unsigned char **src, unsigned char *dest);

// NEON implementations
extern void gf_3vect_dot_prod_neon(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);

// SVE assembly implementations
extern void gf_3vect_dot_prod_sve(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);

// SVE intrinsics implementations (existing)
extern void gf_3vect_dot_prod_sve_intrinsics(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);

// Our new SVE intrinsics implementation
extern void gf_3vect_dot_prod_sve_intrinsics_v2(int len, int vlen, unsigned char *gftbls, unsigned char **src, unsigned char **dest);

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

void run_test(const char *name, 
              void (*func)(int, int, unsigned char*, unsigned char**, unsigned char**),
              int len, int k, unsigned char *g_tbls, unsigned char **src, unsigned char **dest,
              int iterations) {
    
    // Warm up
    func(len, k, g_tbls, src, dest);
    
    // Benchmark
    double start = get_time();
    for (int iter = 0; iter < iterations; iter++) {
        func(len, k, g_tbls, src, dest);
        __asm__ volatile("" : : "r" (dest[0][0]) : "memory");
    }
    double elapsed = get_time() - start;
    
    double bytes_processed = (double)len * k * 3 * iterations;
    printf("%-25s: %.3f seconds (%.1f MB/s)\n", name, elapsed, bytes_processed / elapsed / 1e6);
}

void run_base_test(const char *name, int len, int k, unsigned char *g_tbls, 
                   unsigned char **src, unsigned char **dest_base, int iterations) {
    
    // Warm up
    for (int d = 0; d < 3; d++) {
        gf_vect_dot_prod_base(len, k, &g_tbls[d * k * 32], src, dest_base[d]);
    }
    
    // Benchmark
    double start = get_time();
    for (int iter = 0; iter < iterations; iter++) {
        for (int d = 0; d < 3; d++) {
            gf_vect_dot_prod_base(len, k, &g_tbls[d * k * 32], src, dest_base[d]);
        }
        __asm__ volatile("" : : "r" (dest_base[0][0]) : "memory");
    }
    double elapsed = get_time() - start;
    
    double bytes_processed = (double)len * k * 3 * iterations;
    printf("%-25s: %.3f seconds (%.1f MB/s)\n", name, elapsed, bytes_processed / elapsed / 1e6);
}

int main() {
    const int len = 8192;
    const int k = 6;      // Source vectors
    const int rows = 3;   // Destination vectors
    const int iterations = 2000;
    
    printf("Comprehensive Performance Comparison\n");
    printf("====================================\n");
    printf("Data size: %d bytes\n", len);
    printf("Source vectors: %d\n", k);
    printf("Destination vectors: %d\n", rows);
    printf("Iterations: %d\n", iterations);
    printf("Total computation per test: %.1f MB\n\n", (double)len * k * rows * iterations / 1e6);
    
    // Allocate data
    unsigned char *src[k];
    unsigned char *dest_base[rows];
    unsigned char *dest_neon[rows];
    unsigned char *dest_sve_asm[rows];
    unsigned char *dest_sve_intrinsics[rows];
    unsigned char *dest_sve_v2[rows];
    unsigned char *encode_matrix;
    unsigned char *g_tbls;
    
    // Initialize source data
    srand(42);
    for (int i = 0; i < k; i++) {
        src[i] = malloc(len);
        for (int j = 0; j < len; j++) {
            src[i][j] = rand() & 0xFF;
        }
    }
    
    // Initialize destination arrays
    for (int i = 0; i < rows; i++) {
        dest_base[i] = malloc(len);
        dest_neon[i] = malloc(len);
        dest_sve_asm[i] = malloc(len);
        dest_sve_intrinsics[i] = malloc(len);
        dest_sve_v2[i] = malloc(len);
    }
    
    // Create encoding matrix
    encode_matrix = malloc(rows * k);
    for (int i = 0; i < rows * k; i++) {
        encode_matrix[i] = (rand() & 0xFE) | 1;  // Non-zero
    }
    
    // Initialize GF tables
    g_tbls = malloc(rows * k * 32);
    ec_init_tables(k, rows, encode_matrix, g_tbls);
    
    // Verify all implementations produce the same results
    printf("Verifying correctness...\n");
    
    // Base implementation
    for (int d = 0; d < rows; d++) {
        gf_vect_dot_prod_base(len, k, &g_tbls[d * k * 32], src, dest_base[d]);
    }
    
    // Test other implementations
    gf_3vect_dot_prod_neon(len, k, g_tbls, src, dest_neon);
    gf_3vect_dot_prod_sve(len, k, g_tbls, src, dest_sve_asm);
    gf_3vect_dot_prod_sve_intrinsics(len, k, g_tbls, src, dest_sve_intrinsics);
    gf_3vect_dot_prod_sve_intrinsics_v2(len, k, g_tbls, src, dest_sve_v2);
    
    // Check correctness
    int total_errors = 0;
    const char* impl_names[] = {"NEON", "SVE Assembly", "SVE Intrinsics", "SVE Intrinsics V2"};
    unsigned char** impl_results[] = {dest_neon, dest_sve_asm, dest_sve_intrinsics, dest_sve_v2};
    
    for (int impl = 0; impl < 4; impl++) {
        int errors = 0;
        for (int d = 0; d < rows && errors < 5; d++) {
            for (int i = 0; i < len && errors < 5; i++) {
                if (dest_base[d][i] != impl_results[impl][d][i]) {
                    printf("ERROR %s: Mismatch at [%d][%d]: base=0x%02x, impl=0x%02x\n", 
                           impl_names[impl], d, i, dest_base[d][i], impl_results[impl][d][i]);
                    errors++;
                    total_errors++;
                }
            }
        }
        if (errors == 0) {
            printf("%s: PASS\n", impl_names[impl]);
        }
    }
    
    if (total_errors > 0) {
        printf("\nERROR: Found %d total mismatches. Aborting performance test.\n", total_errors);
        return 1;
    }
    
    printf("\nAll implementations match! Running performance tests...\n\n");
    
    // Performance comparison
    printf("Performance Results:\n");
    printf("--------------------\n");
    
    run_base_test("Base (scalar)", len, k, g_tbls, src, dest_base, iterations);
    run_test("NEON", gf_3vect_dot_prod_neon, len, k, g_tbls, src, dest_neon, iterations);
    run_test("SVE Assembly", gf_3vect_dot_prod_sve, len, k, g_tbls, src, dest_sve_asm, iterations);
    run_test("SVE Intrinsics (orig)", gf_3vect_dot_prod_sve_intrinsics, len, k, g_tbls, src, dest_sve_intrinsics, iterations);
    run_test("SVE Intrinsics V2", gf_3vect_dot_prod_sve_intrinsics_v2, len, k, g_tbls, src, dest_sve_v2, iterations);
    
    // Cleanup
    for (int i = 0; i < k; i++) free(src[i]);
    for (int i = 0; i < rows; i++) {
        free(dest_base[i]);
        free(dest_neon[i]);
        free(dest_sve_asm[i]);
        free(dest_sve_intrinsics[i]);
        free(dest_sve_v2[i]);
    }
    free(encode_matrix);
    free(g_tbls);
    
    return 0;
}
