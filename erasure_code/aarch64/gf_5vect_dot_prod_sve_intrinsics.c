#include <arm_sve.h>
#include <stdint.h>

__attribute__((target("+sve")))
void gf_5vect_dot_prod_sve_intrinsics(int len, int vlen, unsigned char *gftbls, 
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
        svuint8_t dest_acc2 = svdup_u8(0);
        svuint8_t dest_acc3 = svdup_u8(0);
        svuint8_t dest_acc4 = svdup_u8(0);
        
        for (int v = 0; v < vlen; v++) {
            svuint8_t src_data = svld1_u8(pg, &src[v][pos]);
            svuint8_t src_lo = svand_x(pg, src_data, mask0f);
            svuint8_t src_hi = svlsr_x(pg, src_data, 4);
            
            for (int d = 0; d < 5; d++) {
                unsigned char *tbl_base = &gftbls[d * vlen * 32 + v * 32];
                svuint8_t tbl_lo = svld1_u8(svptrue_b8(), tbl_base);
                svuint8_t tbl_hi = svld1_u8(svptrue_b8(), tbl_base + 16);
                svuint8_t gf_lo = svtbl_u8(tbl_lo, src_lo);
                svuint8_t gf_hi = svtbl_u8(tbl_hi, src_hi);
                svuint8_t gf_result = sveor_x(pg, gf_lo, gf_hi);
                
                if (d == 0) dest_acc0 = sveor_x(pg, dest_acc0, gf_result);
                else if (d == 1) dest_acc1 = sveor_x(pg, dest_acc1, gf_result);
                else if (d == 2) dest_acc2 = sveor_x(pg, dest_acc2, gf_result);
                else if (d == 3) dest_acc3 = sveor_x(pg, dest_acc3, gf_result);
                else dest_acc4 = sveor_x(pg, dest_acc4, gf_result);
            }
        }
        
        svst1_u8(pg, &dest[0][pos], dest_acc0);
        svst1_u8(pg, &dest[1][pos], dest_acc1);
        svst1_u8(pg, &dest[2][pos], dest_acc2);
        svst1_u8(pg, &dest[3][pos], dest_acc3);
        svst1_u8(pg, &dest[4][pos], dest_acc4);
        
        pos += svcntb();
    }
}
