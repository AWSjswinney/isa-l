# Generalized SVE Dot Product Implementation

## Overview

This implementation provides a generalized SVE (Scalable Vector Extension) function for computing 1-7 vector dot products simultaneously in GF(2^8), with optimized wrapper functions that allow the compiler to inline and optimize for specific vector counts.

## Files

- `erasure_code/aarch64/gf_nvect_dot_prod_sve_intrinsics_v2.c` - Main implementation
- `test_gf_nvect_sve_simple.c` - Correctness test
- `perf_test_gf_nvect_sve.c` - Performance benchmark

## Key Features

### 1. Generic Core Function
```c
static inline void gf_nvect_dot_prod_sve_generic(int len, int vlen, unsigned char *gftbls, 
                                                 unsigned char **src, unsigned char **dest, int nvect)
```

- Handles 1-7 destination vectors with compile-time optimization
- Uses switch statements for efficient accumulator management
- Compiler can optimize away unused cases when `nvect` is constant

### 2. Optimized Wrapper Functions
```c
void gf_1vect_dot_prod_sve_intrinsics_v2(...);  // Single vector
void gf_2vect_dot_prod_sve_intrinsics_v2(...);  // 2 vectors
// ... up to 7 vectors
```

- Each wrapper calls the generic function with a compile-time constant
- Enables aggressive compiler optimization and inlining
- Maintains clean API for different use cases

### 3. Performance Optimizations

- **SVE Scalability**: Automatically adapts to different SVE vector lengths
- **Nibble Table Lookups**: Uses efficient 4-bit table lookups for GF multiplication
- **Parallel Processing**: Processes multiple destination vectors simultaneously
- **Compiler Optimization**: Switch statements allow dead code elimination

## Performance Results

Testing on AArch64 with SVE support:

| Implementation | 3-Vector Throughput | 5-Vector Throughput | Speedup |
|----------------|--------------------|--------------------|---------|
| Scalar Base    | 130.9 MB/s         | 130.5 MB/s         | 1.0x    |
| SVE Optimized  | 17,239 MB/s        | 15,652 MB/s        | ~130x   |

## Usage Example

```c
#include "gf_nvect_dot_prod_sve_intrinsics_v2.c"

// For 3 destination vectors
unsigned char *src[vlen];
unsigned char *dest[3];
unsigned char *gftbls;

// Initialize data and tables...

// Single function call computes all 3 dot products
gf_3vect_dot_prod_sve_intrinsics_v2(len, vlen, gftbls, src, dest);
```

## Compilation

```bash
gcc -march=armv8-a+sve -O2 your_code.c gf_nvect_dot_prod_sve_intrinsics_v2.c
```

## Technical Details

### SVE Vector Management
- Uses individual `svuint8_t` variables instead of arrays (SVE limitation)
- Switch-based initialization and storage for different vector counts
- Predicate-based processing handles variable-length vectors

### GF(2^8) Arithmetic
- Nibble-based table lookups for efficient multiplication
- 32-byte tables per coefficient (16 bytes low + 16 bytes high nibbles)
- XOR-based accumulation for GF addition

### Compiler Optimization
- `always_inline` attribute ensures inlining of generic function
- Compile-time constants enable dead code elimination
- Switch statements optimize better than loops for small, fixed counts

## Benefits Over Existing Implementation

1. **Single Source**: One generic implementation instead of 7 separate files
2. **Maintainability**: Easier to update and debug
3. **Consistency**: Identical algorithm across all vector counts
4. **Flexibility**: Easy to extend to more vector counts if needed
5. **Performance**: Compiler can optimize aggressively with compile-time constants
