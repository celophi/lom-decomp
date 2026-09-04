typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
#define M2C_FIELD(base, type, off) (*(type)((u8 *)(base) + (off)))
extern u8 D_800C6720;

void *func_800514D8(void *arg0, s32 *arg1, s32 arg2)
{
    s32 sp0;
    s32 temp_a1;
    s32 temp_a3;
    s32 temp_s0;
    s32 temp_v1;
    s32 var_s4;
    s32 var_v0_3;
    s32 var_a2;
    s32 var_fp;
    s32 var_s3;
    s32 var_s7;
    s32 var_t2;
    s32 var_t3;
    s32 var_t4;
    s32 var_t5;
    s32 var_t7;
    s32 var_t8;
    s32 var_t9;
    u8 var_s1;
    u8 var_v0_2;
    u32 mask24;
    void *temp_a0;
    void *temp_t1;
    void *var_a0;
    void *var_v0;

    var_a0 = arg0;
    temp_t1 = (arg2 * 0x18) + &D_800C6720;
    if (arg2 == 2) {
        var_s4 = 2;
        var_s7 = M2C_FIELD(temp_t1, u16 *, 0x14) - 1;
        var_fp = M2C_FIELD(temp_t1, u16 *, 0x16) - 1;
    } else {
        var_s4 = 1;
        var_s7 = M2C_FIELD(temp_t1, u16 *, 0x14);
        var_fp = M2C_FIELD(temp_t1, u16 *, 0x16);
    }
    var_s1 = M2C_FIELD(temp_t1, u8 *, 3);
    var_v0 = var_a0;
    if (var_s4 != 0) {
        do { do { do { do { do { do { do { do { mask24 = 0xFFFFFF; } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0);
        do {
            var_s3 = var_fp;
            var_t8 = 0x100;
            do { var_t7 = M2C_FIELD(temp_t1, u16 *, 0x12); } while (0);
            var_a2 = M2C_FIELD(temp_t1, u16 *, 6);
            sp0 = (s32)M2C_FIELD(temp_t1, u16 *, 0xE);
            if ((s32)var_t7 < 0x101) var_t8 = var_t7;
            do {
                var_t9 = var_s7;
                var_t2 = M2C_FIELD(temp_t1, u16 *, 0x10);
                var_t4 = M2C_FIELD(temp_t1, u16 *, 4);
                var_t5 = M2C_FIELD(temp_t1, u16 *, 0xC);
                var_t3 = 0x80;
                if ((s32)var_t2 < 0x81) var_t3 = var_t2;
                temp_s0 = (s32)(var_a2 & 0x100) >> 4;
                temp_a1 = (var_a2 & 0x200) * 4;
                do {
                    M2C_FIELD(var_a0, s8 *, 3) = 4;
                    M2C_FIELD(var_a0, u8 *, 7) = 0x64U;
                    M2C_FIELD(var_a0, u8 *, 6) = var_s1;
                    M2C_FIELD(var_a0, u8 *, 5) = var_s1;
                    M2C_FIELD(var_a0, u8 *, 4) = var_s1;
                    if (M2C_FIELD(temp_t1, u8 *, 2) != 0) {
                        var_v0_2 = M2C_FIELD(var_a0, u8 *, 7) | 2;
                    } else {
                        var_v0_2 = M2C_FIELD(var_a0, u8 *, 7) & 0xFD;
                    }
                    M2C_FIELD(var_a0, u8 *, 7) = var_v0_2;
                    M2C_FIELD(var_a0, u16 *, 8) = var_t9;
                    M2C_FIELD(var_a0, u16 *, 0xA) = var_s3;
                    M2C_FIELD(var_a0, s8 *, 0xC) = (s8)var_t5;
                    M2C_FIELD(var_a0, u8 *, 0xD) = (u8)sp0;
                    M2C_FIELD(var_a0, u16 *, 0x10) = var_t3;
                    M2C_FIELD(var_a0, u16 *, 0x12) = var_t8;
                    do {
                        M2C_FIELD(var_a0, s16 *, 0xE) = (s16)((M2C_FIELD(temp_t1, u16 *, 0xA) << 6) | (((u16)M2C_FIELD(temp_t1, u16 *, 8) >> 4) & 0x3F));
                    } while (0);
                    M2C_FIELD(var_a0, s32 *, 0) = (s32)((M2C_FIELD(var_a0, s32 *, 0) & 0xFF000000) | (*arg1 & mask24));
                    *arg1 = (*arg1 & 0xFF000000) | ((s32)var_a0 & mask24);
                    var_a0 += 0x14;
                    M2C_FIELD(var_a0, s8 *, 3) = 1;
                    temp_v1 = (M2C_FIELD(temp_t1, u8 *, 0) & 3) << 7;
                    temp_a3 = (s32)(var_t4 & 0x3FF) >> 6;
                    temp_a0 = var_a0;
                    if ((arg2 != 2) || (var_s4 != 1)) {
                        var_v0_3 = temp_v1 | ((M2C_FIELD(temp_t1, u8 *, 1) & 3) << 5) | temp_s0 | temp_a3 | temp_a1;
                        var_v0_3 |= 0xE1000000;
                    } else {
                        var_v0_3 = temp_v1 | temp_s0 | temp_a3 | temp_a1;
                        var_v0_3 |= 0xE1000000;
                    }
                    M2C_FIELD(var_a0, s32 *, 4) = var_v0_3;
                    var_a0 = temp_a0 + 8;
                    var_t2 -= var_t3;
                    M2C_FIELD(temp_a0, s32 *, 0) = (s32)((M2C_FIELD(temp_a0, s32 *, 0) & 0xFF000000) | (*arg1 & mask24));
                    *arg1 = (*arg1 & 0xFF000000) | ((s32)temp_a0 & mask24);
                    if (var_t2 == 0) break;
                    var_t5 ^= 0x80;
                    if (M2C_FIELD(temp_t1, u8 *, 0) == 0) {
                        var_t4 += 0x20;
                    } else {
                        var_t4 += 0x40;
                        var_t5 = 0;
                    }
                    var_t3 = 0x80;
                    if ((s32)var_t2 < 0x81) var_t3 = var_t2;
                    var_t9 += 0x80;
                } while (1);
                var_t7 -= var_t8;
                var_a2 += 0x100;
                if (var_t7 != 0) {
                    sp0 = 0;
                    var_t8 = 0x100;
                    if ((s32)var_t7 < 0x101) var_t8 = var_t7;
                    var_s3 += 0x100;
                }
            } while (var_t7 != 0);
            var_s7 += 2;
            var_fp += 2;
            var_s4 -= 1;
            var_s1 = 0;
        } while (var_s4 != 0);
        var_v0 = var_a0;
    }
    return var_v0;
}
