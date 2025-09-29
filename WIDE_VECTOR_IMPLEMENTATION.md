# Wide Vector Dispatch Implementation

## Overview

This implementation adds support for wide vector unit optimizations in ISA-L's aarch64 erasure code functions. The approach creates additional "_wide" variants of NEON functions that are optimized for systems with wide vector execution units, while maintaining backward compatibility with existing implementations.

## Key Changes

### 1. Wide NEON Variant Implementation

- **File**: `erasure_code/aarch64/gf_vect_dot_prod_neon.S`
- **Function**: `gf_vect_dot_prod_neon_wide`
- **Optimizations**:
  - Removed prefetch instructions that don't improve performance on wide units
  - Reordered dependent eor instruction pairs for better pipeline utilization
  - Grouped independent operations to reduce pipeline stalls
  - Separated dependent instructions to allow parallel execution

### 2. Enhanced Dispatcher Logic

- **File**: `erasure_code/aarch64/ec_aarch64_dispatcher.c`
- **Features**:
  - Maintains existing SVE width detection for 128-bit SVE fallback to NEON
  - Adds `has_wide_vector_units()` function for detecting wide vector capabilities
  - Environment variable control via `ISAL_USE_WIDE_NEON=1`
  - Preserves all existing dispatch behavior for backward compatibility

### 3. Header Updates

- **File**: `include/erasure_code.h`
- **Addition**: Public declaration for `gf_vect_dot_prod_neon_wide` function

## Architecture

```
Dispatcher Logic:
├── SVE Available?
│   ├── Yes: SVE Vector Length > 128-bit? → Use SVE
│   └── No:  128-bit SVE? → Check Wide Vector Units
│       ├── Wide Units Available? → Use gf_vect_dot_prod_neon_wide
│       └── Regular Units → Use gf_vect_dot_prod_neon
└── NEON Available?
    ├── Wide Units Available? → Use gf_vect_dot_prod_neon_wide
    └── Regular Units → Use gf_vect_dot_prod_neon
```

## Performance Optimizations in Wide Variant

### Original NEON Implementation
```assembly
eor v_p0.16b, v_data_0_lo.16b, v_p0.16b
eor v_p0.16b, v_p0.16b, v_data_0_hi.16b
eor v_p1.16b, v_data_1_lo.16b, v_p1.16b
eor v_p1.16b, v_p1.16b, v_data_1_hi.16b
```

### Wide NEON Implementation
```assembly
eor v_p0.16b, v_data_0_lo.16b, v_p0.16b
eor v_p1.16b, v_data_1_lo.16b, v_p1.16b
eor v_p2.16b, v_data_2_lo.16b, v_p2.16b
eor v_p3.16b, v_data_3_lo.16b, v_p3.16b
eor v_p0.16b, v_p0.16b, v_data_0_hi.16b
eor v_p1.16b, v_p1.16b, v_data_1_hi.16b
eor v_p2.16b, v_p2.16b, v_data_2_hi.16b
eor v_p3.16b, v_p3.16b, v_data_3_hi.16b
```

The wide variant groups independent operations together, allowing better instruction-level parallelism on cores with wide vector execution units.

## Testing

### Environment Variable Control
```bash
# Use regular NEON
./erasure_code_test

# Use wide NEON variant
ISAL_USE_WIDE_NEON=1 ./erasure_code_test
```

### Verification
- All existing tests pass with both variants
- Functional equivalence verified between regular and wide implementations
- No performance regression on narrow vector units

## Future Extensions

This framework can be extended to support wide variants for other functions:

1. `gf_2vect_dot_prod_neon_wide`
2. `gf_3vect_dot_prod_neon_wide`
3. `gf_4vect_dot_prod_neon_wide`
4. `gf_5vect_dot_prod_neon_wide`

Each would follow the same pattern:
- Copy optimized implementation from scheduling cleanup branch
- Add as "_wide" variant in same source file
- Update dispatcher to include function in wide vector detection

## CPU Detection Enhancement

For production use, the `has_wide_vector_units()` function could be enhanced to detect specific CPU models:

```c
static inline int
has_wide_vector_units(void)
{
    // Environment variable override for testing
    const char *env_wide = getenv("ISAL_USE_WIDE_NEON");
    if (env_wide && env_wide[0] == '1') {
        return 1;
    }
    
    // Check CPU part numbers for known wide vector units
    // Examples: Neoverse V1/V2, Cortex-X series, etc.
    
    return 0;
}
```

## Benefits

1. **Performance**: Optimized instruction scheduling for wide vector units
2. **Compatibility**: No impact on existing narrow vector unit performance
3. **Flexibility**: Runtime selection based on hardware capabilities
4. **Maintainability**: Both variants in same source files for easier maintenance
5. **Testability**: Environment variable allows easy testing of both paths
