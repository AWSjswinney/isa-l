# ISA-L N-Vector Dot Product Benchmark

Comprehensive performance benchmark for all n-vector dot product implementations in ISA-L.

## Overview

This benchmark tests and compares performance across all available implementations:

- **Base (scalar)**: Reference C implementation
- **NEON**: Hand-optimized NEON assembly
- **SVE Assembly**: Hand-optimized SVE assembly  
- **SVE Intrinsics**: Original SVE intrinsics implementation
- **SVE V2**: Generic SVE intrinsics with compile-time optimization
- **SVE V3**: Simplified SVE intrinsics with 4x unrolling and predicate masking

## Building

```bash
make                # Build benchmark
make test          # Run 3-vector benchmark
make test-all      # Run all n-vector benchmarks (1-5)
make clean         # Clean build files
```

## Usage

```bash
./nvect_benchmark [n]    # Run benchmark for n vectors (1-7)
./nvect_benchmark        # Run all benchmarks
```

## Performance Results Summary

Based on AArch64 with 128-bit SVE support:

| N-Vectors | Best Implementation | Throughput (MB/s) | vs NEON | vs Base |
|-----------|--------------------|--------------------|---------|---------|
| 1 | NEON | 24,813 | 1.0x | 24.8x |
| 2 | NEON | 31,139 | 1.0x | 31.3x |
| 3 | NEON | 32,175 | 1.0x | 32.3x |
| 4 | NEON | 32,597 | 1.0x | 32.8x |
| 5 | NEON | 32,003 | 1.0x | 32.1x |
| 6 | SVE V2 | 31,324 | 102.2%* | 31.3x |
| 7 | SVE V2 | 32,613 | 103.2%* | 32.7x |

*NEON uses combinations: 6-vector = 5+1 calls, 7-vector = 5+2 calls

### SVE V3 Performance

SVE V3 (simplified with 4x unrolling) achieves excellent performance:

| N-Vectors | SVE V3 (MB/s) | vs NEON | vs SVE V2 |
|-----------|---------------|---------|-----------|
| 1 | 24,317 | 98.0% | +16.4% |
| 2 | 29,499 | 94.7% | +16.9% |
| 3 | 31,192 | 97.0% | +8.0% |
| 4 | 32,380 | 99.3% | +8.9% |
| 5 | 31,905 | 99.7% | +5.3% |
| 6 | 29,887 | 97.5% | -4.6% |
| 7 | 31,902 | 101.0% | -2.2% |

## Key Findings

1. **NEON leads** on 128-bit SVE platforms for 1-5 vectors due to maturity and optimization
2. **SVE V2 excels** at 6-7 vectors, outperforming NEON combinations by 2-3%
3. **SVE V3 is competitive** - achieves 94-101% of NEON performance across all vector counts
4. **SVE V3 outperforms other SVE implementations** by 5-17% for smaller vector counts
5. **All implementations scale well** with increasing vector counts up to 7 vectors
6. **Correctness verified** - all implementations produce identical results
7. **NEON combinations** (5+1, 5+2) show the overhead of multiple function calls vs single SVE calls

## Technical Notes

- Compiled with `-march=armv8-a+sve -O3`
- Tests use 8192-byte data size with 6 source vectors
- 2000 iterations per benchmark for stable timing
- Memory barriers prevent compiler optimization of benchmarks
- Results include both performance and correctness verification
- 6-vector NEON uses 5+1 combination, 7-vector uses 5+2 combination (matching ISA-L implementation)
