#include <arm_sve.h>
#include <stdint.h>

__attribute__((target("+sve"), always_inline))
static inline void gf_nvect_dot_prod_sve_generic(int len, int vlen, unsigned char *gftbls, 
                                                 unsigned char **src, unsigned char **dest, int nvect)
{
    if (len < 16) return;
    
    const svuint8_t mask0f = svdup_u8(0x0f);
    int pos = 0;
    
    while (pos < len) {
        svbool_t pg = svwhilelt_b8_s32(pos, len);
        if (!svptest_any(svptrue_b8(), pg)) break;
        
        // Use individual variables instead of array
        svuint8_t dest_acc0, dest_acc1, dest_acc2, dest_acc3, dest_acc4, dest_acc5, dest_acc6;
        
        switch (nvect) {
            case 7: dest_acc6 = svdup_u8(0); // fallthrough
            case 6: dest_acc5 = svdup_u8(0); // fallthrough  
            case 5: dest_acc4 = svdup_u8(0); // fallthrough
            case 4: dest_acc3 = svdup_u8(0); // fallthrough
            case 3: dest_acc2 = svdup_u8(0); // fallthrough
            case 2: dest_acc1 = svdup_u8(0); // fallthrough
            case 1: dest_acc0 = svdup_u8(0); break;
        }
        
        for (int v = 0; v < vlen; v++) {
            svuint8_t src_data = svld1_u8(pg, &src[v][pos]);
            svuint8_t src_lo = svand_x(pg, src_data, mask0f);
            svuint8_t src_hi = svlsr_x(pg, src_data, 4);
            
            for (int d = 0; d < nvect; d++) {
                unsigned char *tbl_base = &gftbls[d * vlen * 32 + v * 32];
                svuint8_t tbl_lo = svld1_u8(svptrue_b8(), tbl_base);
                svuint8_t tbl_hi = svld1_u8(svptrue_b8(), tbl_base + 16);
                svuint8_t gf_lo = svtbl_u8(tbl_lo, src_lo);
                svuint8_t gf_hi = svtbl_u8(tbl_hi, src_hi);
                svuint8_t gf_result = sveor_x(pg, gf_lo, gf_hi);
                
                switch (d) {
                    case 0: dest_acc0 = sveor_x(pg, dest_acc0, gf_result); break;
                    case 1: dest_acc1 = sveor_x(pg, dest_acc1, gf_result); break;
                    case 2: dest_acc2 = sveor_x(pg, dest_acc2, gf_result); break;
                    case 3: dest_acc3 = sveor_x(pg, dest_acc3, gf_result); break;
                    case 4: dest_acc4 = sveor_x(pg, dest_acc4, gf_result); break;
                    case 5: dest_acc5 = sveor_x(pg, dest_acc5, gf_result); break;
                    case 6: dest_acc6 = sveor_x(pg, dest_acc6, gf_result); break;
                }
            }
        }
        
        switch (nvect) {
            case 7: svst1_u8(pg, &dest[6][pos], dest_acc6); // fallthrough
            case 6: svst1_u8(pg, &dest[5][pos], dest_acc5); // fallthrough
            case 5: svst1_u8(pg, &dest[4][pos], dest_acc4); // fallthrough
            case 4: svst1_u8(pg, &dest[3][pos], dest_acc3); // fallthrough
            case 3: svst1_u8(pg, &dest[2][pos], dest_acc2); // fallthrough
            case 2: svst1_u8(pg, &dest[1][pos], dest_acc1); // fallthrough
            case 1: svst1_u8(pg, &dest[0][pos], dest_acc0); break;
        }
        
        pos += svcntb();
    }
}

// Optimized wrapper functions with compile-time constants
__attribute__((target("+sve")))
void gf_1vect_dot_prod_sve_intrinsics_v2(int len, int vlen, unsigned char *gftbls, 
                                         unsigned char **src, unsigned char *dest)
{
    unsigned char *dest_array[1] = {dest};
    gf_nvect_dot_prod_sve_generic(len, vlen, gftbls, src, dest_array, 1);
}

__attribute__((target("+sve")))
void gf_2vect_dot_prod_sve_intrinsics_v2(int len, int vlen, unsigned char *gftbls, 
                                         unsigned char **src, unsigned char **dest)
{
    gf_nvect_dot_prod_sve_generic(len, vlen, gftbls, src, dest, 2);
}

__attribute__((target("+sve")))
void gf_3vect_dot_prod_sve_intrinsics_v2(int len, int vlen, unsigned char *gftbls, 
                                         unsigned char **src, unsigned char **dest)
{
    gf_nvect_dot_prod_sve_generic(len, vlen, gftbls, src, dest, 3);
}

__attribute__((target("+sve")))
void gf_4vect_dot_prod_sve_intrinsics_v2(int len, int vlen, unsigned char *gftbls, 
                                         unsigned char **src, unsigned char **dest)
{
    gf_nvect_dot_prod_sve_generic(len, vlen, gftbls, src, dest, 4);
}

__attribute__((target("+sve")))
void gf_5vect_dot_prod_sve_intrinsics_v2(int len, int vlen, unsigned char *gftbls, 
                                         unsigned char **src, unsigned char **dest)
{
    gf_nvect_dot_prod_sve_generic(len, vlen, gftbls, src, dest, 5);
}

__attribute__((target("+sve")))
void gf_6vect_dot_prod_sve_intrinsics_v2(int len, int vlen, unsigned char *gftbls, 
                                         unsigned char **src, unsigned char **dest)
{
    gf_nvect_dot_prod_sve_generic(len, vlen, gftbls, src, dest, 6);
}

__attribute__((target("+sve")))
void gf_7vect_dot_prod_sve_intrinsics_v2(int len, int vlen, unsigned char *gftbls, 
                                         unsigned char **src, unsigned char **dest)
{
    gf_nvect_dot_prod_sve_generic(len, vlen, gftbls, src, dest, 7);
}
