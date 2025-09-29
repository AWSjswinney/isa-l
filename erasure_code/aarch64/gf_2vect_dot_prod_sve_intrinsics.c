#include <arm_sve.h>
#include <stdint.h>

__attribute__((target("+sve")))
void gf_2vect_dot_prod_sve_intrinsics(int len, int vlen, unsigned char *gftbls, 
                                      unsigned char **src, unsigned char **dest)
{
    if (len < 16) return;
    
    const svuint8_t mask0f = svdup_u8(0x0f);
    int pos = 0;
    
    while (pos < len) {
        svbool_t pg = svwhilelt_b8_s32(pos, len);
        if (!svptest_any(svptrue_b8(), pg)) break;
        
        svuint8_t dest_acc0 = svdup_u8(0);
        svuint8_t dest_acc1 = svdup_u8(0);
        
        for (int v = 0; v < vlen; v++) {
            svuint8_t src_data = svld1_u8(pg, &src[v][pos]);
            svuint8_t src_lo = svand_x(pg, src_data, mask0f);
            svuint8_t src_hi = svlsr_x(pg, src_data, 4);
            
            // Destination 0
            unsigned char *tbl_base0 = &gftbls[0 * vlen * 32 + v * 32];
            svuint8_t tbl_lo0 = svld1_u8(svptrue_b8(), tbl_base0);
            svuint8_t tbl_hi0 = svld1_u8(svptrue_b8(), tbl_base0 + 16);
            svuint8_t gf_lo0 = svtbl_u8(tbl_lo0, src_lo);
            svuint8_t gf_hi0 = svtbl_u8(tbl_hi0, src_hi);
            svuint8_t gf_result0 = sveor_x(pg, gf_lo0, gf_hi0);
            dest_acc0 = sveor_x(pg, dest_acc0, gf_result0);
            
            // Destination 1
            unsigned char *tbl_base1 = &gftbls[1 * vlen * 32 + v * 32];
            svuint8_t tbl_lo1 = svld1_u8(svptrue_b8(), tbl_base1);
            svuint8_t tbl_hi1 = svld1_u8(svptrue_b8(), tbl_base1 + 16);
            svuint8_t gf_lo1 = svtbl_u8(tbl_lo1, src_lo);
            svuint8_t gf_hi1 = svtbl_u8(tbl_hi1, src_hi);
            svuint8_t gf_result1 = sveor_x(pg, gf_lo1, gf_hi1);
            dest_acc1 = sveor_x(pg, dest_acc1, gf_result1);
        }
        
        svst1_u8(pg, &dest[0][pos], dest_acc0);
        svst1_u8(pg, &dest[1][pos], dest_acc1);
        
        pos += svcntb();
    }
}
