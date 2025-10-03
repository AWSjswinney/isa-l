CC = gcc
CFLAGS = -march=armv8-a+sve -O3 -I.. -I../include
LDFLAGS = 

# ISA-L source files
ISAL_BASE = ../erasure_code/ec_base.c ../erasure_code/ec_base_aliases.c

# NEON implementations
NEON_SRCS = ../erasure_code/aarch64/gf_vect_dot_prod_neon.S \
            ../erasure_code/aarch64/gf_2vect_dot_prod_neon.S \
            ../erasure_code/aarch64/gf_3vect_dot_prod_neon.S \
            ../erasure_code/aarch64/gf_4vect_dot_prod_neon.S \
            ../erasure_code/aarch64/gf_5vect_dot_prod_neon.S

# SVE Assembly implementations
SVE_ASM_SRCS = ../erasure_code/aarch64/gf_vect_dot_prod_sve.S \
               ../erasure_code/aarch64/gf_2vect_dot_prod_sve.S \
               ../erasure_code/aarch64/gf_3vect_dot_prod_sve.S \
               ../erasure_code/aarch64/gf_4vect_dot_prod_sve.S \
               ../erasure_code/aarch64/gf_5vect_dot_prod_sve.S

# SVE Intrinsics implementations
SVE_INTRINSICS_SRCS = ../erasure_code/aarch64/gf_vect_dot_prod_sve_intrinsics.c \
                      ../erasure_code/aarch64/gf_2vect_dot_prod_sve_intrinsics.c \
                      ../erasure_code/aarch64/gf_3vect_dot_prod_sve_intrinsics.c \
                      ../erasure_code/aarch64/gf_4vect_dot_prod_sve_intrinsics.c \
                      ../erasure_code/aarch64/gf_5vect_dot_prod_sve_intrinsics.c

# SVE V2 and V3 implementations
SVE_V2_SRCS = ../erasure_code/aarch64/gf_nvect_dot_prod_sve_intrinsics_v2.c
SVE_V3_SRCS = ../erasure_code/aarch64/gf_nvect_dot_prod_sve_intrinsics_v3.c

ALL_SRCS = $(ISAL_BASE) $(NEON_SRCS) $(SVE_ASM_SRCS) $(SVE_INTRINSICS_SRCS) $(SVE_V2_SRCS) $(SVE_V3_SRCS)

.PHONY: all clean test

all: nvect_benchmark

nvect_benchmark: nvect_benchmark.c $(ALL_SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test: nvect_benchmark
	@echo "Running 3-vector benchmark..."
	./nvect_benchmark 3

test-all: nvect_benchmark
	@echo "Running all n-vector benchmarks..."
	./nvect_benchmark

clean:
	rm -f nvect_benchmark

help:
	@echo "Available targets:"
	@echo "  all        - Build the benchmark"
	@echo "  test       - Run 3-vector benchmark"
	@echo "  test-all   - Run all n-vector benchmarks (1-7)"
	@echo "  clean      - Remove built files"
	@echo ""
	@echo "Usage:"
	@echo "  ./nvect_benchmark [n]  - Run benchmark for n vectors (1-7)"
	@echo "  ./nvect_benchmark      - Run all benchmarks"
