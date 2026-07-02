typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;

typedef unsigned char   undefined;
typedef unsigned char   undefined1;
typedef unsigned short  undefined2;
typedef unsigned int    undefined4;

typedef int             s32;
typedef unsigned int    u32;
typedef unsigned char   u8;
typedef signed char     s8;
typedef unsigned short  u16;
typedef signed short    s16;

extern long ratan2(long y, long x);
extern int rcos(int a);
extern int rsin(int a);

#define NULL 0

s16 func_8005DFAC(void*, s32*, s32, s32);           /* extern */
s32 func_8005E1A8(void*, s32, s32, s32);            /* extern */
extern long SquareRoot0(long a);

typedef struct UnkS16 {
    s16 unk0;
    s16 unk2;
} UnkS16;

typedef struct UnkNode2 {
    u8 pad0[4];
    s32 unk4;
    u8 pad8[8];
    s16 unk10;
    s16 unk12;
    s16 unk14;
} UnkNode2;

typedef struct UnkNode1 {
    struct UnkNode1* unk0;
    UnkNode2* unk4;
    u8 pad8[8];
    void* unk10;
    void* unk14;
    u8 unk18;
    u8 pad19[3];
    s16 unk1C;
    s16 unk1E;
    s16 unk20;
    s16 unk22;
    s32 unk24;
    s32 unk28;
    s32 unk2C;
    s32 unk30;
    s32 unk34;
    s32 unk38;
    u8 pad3C[4];
    s32 unk40;
} UnkNode1;

typedef struct UnkNode3 {
    u8 pad0[0x2C];
    s32 unk2C;
    s16 unk30;
    s16 unk32;
} UnkNode3;

/**
 * @brief Actor/mover state resolved by func_8005B6AC.
 * @note Distinct from field6.c's smaller `Query` probe struct (which has u16
 *       unkC/unk10). Prologue asm proves word loads at 0x4/0x10 and an s16 load
 *       at 0x26, so every field here is its asm-confirmed width.
 */
typedef struct Mover {
    s32 unk0;       /* 0x00 world X (fixed-point, >>8 to screen) */
    s32 unk4;       /* 0x04 world Z/height accumulator */
    s32 unk8;       /* 0x08 world Y */
    s32 unkC;       /* 0x0C dX this frame */
    s32 unk10;      /* 0x10 dZ this frame */
    s32 unk14;      /* 0x14 dY this frame */
    s32 unk18;      /* 0x18 output height delta (zeroed in prologue) */
    void* unk1C;    /* 0x1C selected collision node (-1 = needs search) */
    s32 unk20;      /* 0x20 flags (bit0 sticky-contact) */
    u16 unk24;      /* 0x24 footprint width */
    s16 unk26;      /* 0x26 height bias */
    s32 unk28;      /* 0x28 mode flags (0x30000 gate, low s16 = step size) */
} Mover;

typedef struct Node {
  struct Node *next;
  void *obj;
  u8 pad[8];
  s16 right;
  s16 left;
  s16 bottom;
  s16 top;
  s32 denom1;
  s32 mult1;
  s32 denom2;
  s32 mult2;
  s32 max1;
  s32 min1;
  s32 max2;
  s32 min2;
} Node;

typedef struct {
  UnkNode3* unk0;
  u8 pad4[4];
  UnkNode1* unk8;
  UnkNode1* unkC;
  Node *list;
} FieldScene;

extern FieldScene *g_field_scene;

void func_8005DA7C(void**, void*, s32*, s32*);          /* extern */
void func_80062F48(void*, s32*);                        /* extern */

typedef struct Probe {
    Mover* m;
    s32 x;
    s32 y;
    s16 w;
    s16 h;
} Probe;

/**
 * @see decomp.me (57.91%) https://decomp.me/scratch/N2GNJ
 * @note local objdiff 75.44% (gcc280_g4_noexpanddiv), 2026-07-02 - active
 *       matching scratch is working/func_8005B6AC.c.
 */
s32 func_8005B6AC(Mover* a0) {
    Probe probe;
    s32 sp20;
    s32 sp24;
    FieldScene* sp28;
    void* sp2C;
    u16 sp30;
    u16 sp38;
    u16 sp40;
    s32 sp48;
    s32 sp4C;
    s32 sp50;
    s32 sp54;
    s32 sp58;
    s32 sp5C;
    s32 sp60;
    void** sp64;
    void** sp68;
    s32 sp6C;
    s32 sp70;
    s32 sp74;
    s32 sp78;
    s32 sp7C;
    s32 sp80;
    s32 sp84;
    s32 sp88;
    s32 sp8C;
    u8* spA8;
    s32 spAC;
    s16 temp_a0_12;
    s16 temp_a0_13;
    s32 temp_a0_14;
    s16 temp_a0_20;
    s16 temp_a1_5;
    s16 temp_s0;
    s16 temp_s0_3;
    s16 temp_s0_5;
    s16 temp_s0_6;
    s16 temp_s1;
    s16 temp_s2_3;
    s16 temp_s2_4;
    s32 temp_v0_17;
    s16 temp_v1_11;
    s16 temp_v1_13;
    s16 temp_v1_15;
    s16 temp_v1_18;
    s16 temp_v1_19;
    s32 temp_v1_26;
    s32 temp_v1_4;
    s32 temp_v1_5;
    s16 temp_v1_7;
    s16 temp_v1_8;
    s16 temp_v1_9;
    s32 var_a1;
    s32 var_a3_2;
    s16 var_s0_3;
    s16 var_s1_3;
    s32 e_x_min;
    s32 f_x_min;
    s32 f_x_max;
    s32 f_y_min;
    s32 f_y_max;
    s32 var_s2;
    s16 var_t0_2;
    s16 var_t1_2;
    s16 var_t1_3;
    u8* var_s1;
    u8* var_s1_5;
    u8* var_s1_2;
    u8* var_s1_4;
    u8* var_s2_2;
    u8* var_s2_3;
    u8* var_s2_4;
    u8* var_t3;
    s32 temp_a0;
    s32 temp_a0_10;
    s32 temp_a0_15;
    s32 temp_a0_17;
    s32 temp_a0_19;
    s32 temp_a0_4;
    s32 temp_a0_8;
    s32 temp_a1;
    s32 temp_a1_3;
    s32 temp_a1_4;
    s32 temp_a1_6;
    s32 temp_a1_8;
    s32 temp_a1_9;
    s32 temp_a2_2;
    s32 temp_a3;
    s32 temp_a3_2;
    s32 temp_lo;
    s32 temp_lo_2;
    s32 temp_lo_3;
    s32 temp_lo_4;
    s32 temp_s0_2;
    s32 temp_s0_4;
    s32 temp_s2;
    s32 temp_s2_2;
    s32 temp_s5;
    s32 temp_s7;
    s32 temp_t5;
    s32 temp_t5_2;
    s32 temp_t5_3;
    s32 temp_t6_2;
    s32 temp_v0;
    s32 temp_v0_10;
    s32 temp_v0_12;
    s32 temp_v0_13;
    s32 temp_v0_14;
    s32 temp_v0_15;
    s32 temp_v0_16;
    s32 temp_v0_18;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 temp_v0_5;
    s32 temp_v0_6;
    s32 temp_v0_9;
    s32 temp_v1;
    s32 temp_v1_10;
    s32 temp_v1_12;
    s32 temp_v1_14;
    s32 temp_v1_16;
    s32 temp_v1_17;
    s32 temp_v1_20;
    s32 temp_v1_21;
    s32 temp_v1_22;
    s32 temp_v1_23;
    s32 temp_v1_24;
    s32 temp_v1_25;
    s32 temp_v1_27;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 temp_v1_6;
    s32 var_a0;
    s32 var_a2;
    s32 var_a3;
    s32 ux;
    s32 uy;
    s32 var_s0;
    s32 var_s0_10;
    s32 var_s0_11;
    s32 var_s0_2;
    s32 var_s0_4;
    s32 var_s0_5;
    s32 var_s0_6;
    s32 var_s0_7;
    s32 var_s0_8;
    s32 var_s0_9;
    s32 var_s4_3;
    s32 var_s5;
    s32 var_s7;
    s32 var_t0;
    s32 var_t1;
    s32 var_t5;
    s32 var_t7;
    s32 var_t8;
    s32 var_t9;
    s32 var_v0;
    s32 var_v0_10;
    s32 var_v0_11;
    s32 var_v0_12;
    s32 var_v0_13;
    s32 var_v0_14;
    s32 var_v0_15;
    s32 var_v0_16;
    s32 var_v0_17;
    s32 var_v0_18;
    s32 var_v0_19;
    s32 var_v0_20;
    s32 var_v0_21;
    s32 var_v0_22;
    s32 var_v0_23;
    s32 var_v0_24;
    s32 var_v0_25;
    s32 var_v0_26;
    s32 var_v0_27;
    s32 var_v0_28;
    s32 var_v0_29;
    s32 var_v0_2;
    s32 var_v0_30;
    s32 var_v0_31;
    s32 var_v0_32;
    s32 var_v0_33;
    s32 var_v0_34;
    s32 var_v0_35;
    s32 var_v0_36;
    s32 var_v0_37;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_7;
    s32 var_v0_8;
    s32 var_v0_9;
    s32 var_v1;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 var_v1_4;
    s8* var_s3;
    s8* var_s4;
    s32 temp_a0_16;
    s32 temp_a0_6;
    s32 temp_a1_2;
    s32 temp_a1_7;
    s32 temp_v0_11;
    s32 temp_v0_7;
    s32 temp_v0_8;
    u8 temp_a0_18;
    u8 temp_a0_2;
    u8 temp_a0_9;
    u8* var_s4_2;
    void* temp_a0_11;
    void* temp_a0_3;
    void* temp_a0_5;
    void* temp_a0_7;
    void* temp_a2;
    void* temp_a2_3;
    void* temp_fp;
    void* temp_fp_2;
    void* temp_s3;
    void* temp_s3_2;
    void* temp_s3_3;
    void* temp_s6;
    void* temp_s6_2;
    void* temp_s6_3;
    void* temp_s6_4;
    void* temp_s6_5;
    void* temp_s6_6;
    void* temp_t5_4;
    void* temp_t6;
    void* var_fp;

    sp48 = 0;
    sp4C = 0;
    sp50 = 0;
    sp28 = g_field_scene;
    a0->unk18 = 0;
    var_v0_2 = -a0->unk4 - a0->unk10;
    if (var_v0_2 < 0) {
        var_v0_2 += 0xFF;
    }
    sp30 = (u16) ((u32) var_v0_2 >> 8);
    var_v0_3 = -a0->unk4;
    if (var_v0_3 < 0) {
        var_v0_3 += 0xFF;
    }
    sp38 = (var_v0_3 >> 8) + a0->unk26;
    temp_t6 = sp28->unk8;
    sp2C = temp_t6;
    if (temp_t6 != NULL) {
        if (a0->unk1C == (void* )-1) {
            var_s2 = 0;
            var_v0_4 = a0->unk0;
            var_a3 = 0;
            a0->unk1C = NULL;
            if (var_v0_4 < 0) {
                var_v0_4 += 0xFF;
            }
            probe.x = var_v0_4 >> 8;
            var_v0_5 = a0->unk8;
            if (var_v0_5 < 0) {
                var_v0_5 += 0xFF;
            }
            var_fp = sp2C;
            probe.y = var_v0_5 >> 8;
            ux = (u16) probe.x;
            uy = (u16) probe.y;
            var_t8 = 0;
            while (var_fp != NULL) {
                    temp_s6 = ((UnkNode1*)var_fp)->unk4;
                    if ((((UnkNode1*)var_fp)->unk18 != 0) && ((temp_v0 = (s32) ((UnkNode1*)sp2C)->unk38 >> 8, temp_v1 = ((UnkNode2*)temp_s6)->unk14 + (s16) temp_v0, (temp_v1 == 0)) || (temp_v1 < ((s16) sp30 + a0->unk26)) || (temp_v1 < (s16) sp38))) {
                        temp_a1 = (s32) ((UnkNode1*)sp2C)->unk34 >> 8;
                        temp_a0 = (s32) (((UnkNode1*)sp2C)->unk40 << 8) >> 0x10;
                        temp_v1_2 = ((UnkNode1*)var_fp)->unk22 + temp_a0;
                        if (((s16) uy >= temp_v1_2) && ((((UnkNode1*)var_fp)->unk20 + temp_a0) >= (s16) uy)) {
                            temp_a0_2 = ((u8*)temp_s6)[6];
                            var_s1 = (u8*)((UnkNode1*)var_fp)->unk10 + (((s16) uy - temp_v1_2) * temp_a0_2 * 4);
                            if ((s16) temp_a1 != 0) {
                                var_s0 = temp_a0_2 - 1;
                                if (temp_a0_2 != 0) {
                                    do {
                                        if (((s16) ux < (((UnkS16*)var_s1)->unk0 + (s16) temp_a1)) || ((((UnkS16*)var_s1)->unk2 + (s16) temp_a1) < (s16) ux)) {
                                            var_s1 += 4;
                                        } else {
                                            var_t8 = 1;
                                            break;
                                        }
                                    } while (--var_s0 != -1);
                                }
                            } else {
                                var_s0_2 = temp_a0_2 - 1;
                                if (temp_a0_2 != 0) {
                                    do {
                                        if (((s16) ux < ((UnkS16*)var_s1)->unk0) || (((UnkS16*)var_s1)->unk2 < (s16) ux)) {
                                            var_s1 += 4;
                                        } else {
                                            var_t8 = 1;
                                            break;
                                        }
                                    } while (--var_s0_2 != -1);
                                }
                            }
                            if (var_t8 != 0) {
                                temp_v1_3 = ((UnkNode2*)temp_s6)->unk4 & 3;
                                switch (temp_v1_3) { /* switch 1; irregular */
                                case 0:             /* switch 1 */
                                    if ((((UnkNode2*)temp_s6)->unk14 + (s16) temp_v0) < (s16) sp38) {
                                        if (var_a3 != 0) {
                                            temp_v1_4 = ((UnkNode2*)temp_s6)->unk10 + (s16) temp_v0;
                                            if ((var_s2 + 0x14) < temp_v1_4) {
                                                var_s2 = temp_v1_4;
                                                a0->unk1C = var_fp;
                                            }
                                        } else {
                                            temp_v1_5 = ((UnkNode2*)temp_s6)->unk10 + (s16) temp_v0;
                                            if (temp_v1_5 >= var_s2) {
                                                var_s2 = temp_v1_5;
                                                a0->unk1C = var_fp;
                                            }
                                        }
                                    }
                                    break;
                                case 1:             /* switch 1 */
                                    if ((((UnkNode2*)temp_s6)->unk14 + (s16) temp_v0) < (s16) sp38) {
                                        temp_s0 = func_8005DFAC(var_fp, &probe.x, temp_v0, var_a3);
                                        if (var_a3 != 0) {
                                            var_a3 = 1;
                                            if (var_s2 < temp_s0) {
                                                var_s2 = temp_s0;
                                                a0->unk1C = var_fp;
                                            }
                                        } else {
                                            var_a3 = 1;
                                            if (var_s2 < (temp_s0 + 0x14)) {
                                                var_s2 = temp_s0;
                                                a0->unk1C = var_fp;
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    var_fp = ((UnkNode1*)var_fp)->unk0;
            }
        }
        temp_a0_3 = a0->unk1C;
        if ((temp_a0_3 != NULL) && (temp_a0_3 != (void* )-2)) {
            temp_s6_2 = ((UnkNode1*)temp_a0_3)->unk4;
            if (((UnkNode1*)temp_a0_3)->unk18 != 0) {
                if (((((UnkNode2*)temp_s6_2)->unk4 & 3) == 1) && (((UnkNode2*)temp_s6_2)->unk10 != ((UnkNode2*)temp_s6_2)->unk12)) {
                    probe.x = a0->unkC;
                    probe.y = a0->unk14;
                    func_80062F48(temp_s6_2, &probe.x);
                    a0->unkC = probe.x;
                    sp50 |= 4;
                    a0->unk14 = probe.y;
                }
                if (((UnkNode2*)temp_s6_2)->unk4 & 0x10) {
                    temp_t5 = a0->unkC;
                    sp5C = temp_t5;
                    if (temp_t5 >= 0) {
                        var_v0_6 = temp_t5 >> 1;
                    } else {
                        var_v0_6 = (s32) (sp5C + 1) >> 1;
                    }
                    a0->unkC = var_v0_6;
                    temp_t5_2 = a0->unk14;
                    sp60 = temp_t5_2;
                    if (temp_t5_2 >= 0) {
                        var_v0_7 = temp_t5_2 >> 1;
                    } else {
                        var_v0_7 = (s32) (sp60 + 1) >> 1;
                    }
                    a0->unk14 = var_v0_7;
                    sp50 |= 4;
                }
                if (((UnkNode2*)temp_s6_2)->unk4 & 0x40) {
                    sp50 |= 0x10;
                }
                if (!(a0->unk28 & 0x30000) && ((((UnkNode1*)temp_a0_3)->unk24 != 0) || (((UnkNode1*)temp_a0_3)->unk28 != 0) || (((UnkNode1*)temp_a0_3)->unk2C != 0) || (((UnkNode1*)temp_a0_3)->unk30 != 0))) {
                    a0->unkC = (s32) (a0->unkC + ((UnkNode1*)temp_a0_3)->unk24);
                    temp_s2 = a0->unk10;
                    a0->unk14 = (s32) (a0->unk14 + ((UnkNode1*)temp_a0_3)->unk30);
                    if ((((UnkNode2*)temp_s6_2)->unk4 & 3) == 1) {
                        var_v0_8 = a0->unk0;
                        if (var_v0_8 < 0) {
                            var_v0_8 += 0xFF;
                        }
                        probe.x = var_v0_8 >> 8;
                        var_v0_9 = a0->unk8;
                        if (var_v0_9 < 0) {
                            var_v0_9 += 0xFF;
                        }
                        probe.y = var_v0_9 >> 8;
                        a0->unk10 = (s32) (-((s32) (func_8005DFAC(temp_a0_3, &probe.x, 0, 0) << 0x10) >> 8) - a0->unk4);
                    } else {
                        a0->unk10 = (s32) (temp_s2 + ((UnkNode1*)temp_a0_3)->unk28);
                    }
                    if (temp_s2 != a0->unk10) {
                        sp50 |= 0x30;
                    } else if ((((UnkNode1*)temp_a0_3)->unk24 != 0) || (((UnkNode1*)temp_a0_3)->unk30 != 0)) {
                        sp50 |= 0x10;
                    }
                } else {
                    temp_s3 = sp28->unkC;
                    if ((temp_s3 != NULL) && (temp_s3 != temp_a0_3) && ((((UnkNode1*)temp_s3)->unk24 != 0) || (((UnkNode1*)temp_s3)->unk28 != 0) || (((UnkNode1*)temp_s3)->unk2C != 0) || (((UnkNode1*)temp_s3)->unk30 != 0))) {
                        temp_s2_2 = a0->unk10;
                        a0->unkC = (s32) (a0->unkC + ((UnkNode1*)temp_s3)->unk24);
                        temp_a0_4 = a0->unk0;
                        a0->unk14 = (s32) (a0->unk14 + ((UnkNode1*)temp_s3)->unk30);
                        if (temp_a0_4 >= 0) {
                            var_v0_10 = temp_a0_4 >> 8;
                        } else {
                            var_v0_10 = (s32) (temp_a0_4 + 0xFF) >> 8;
                        }
                        probe.x = var_v0_10;
                        var_v0_11 = a0->unk8;
                        if (var_v0_11 < 0) {
                            var_v0_11 += 0xFF;
                        }
                        probe.y = var_v0_11 >> 8;
                        temp_s0_2 = func_8005DFAC(temp_s3, &probe.x, 0, 0) - ((UnkNode2*)((UnkNode1*)temp_s3)->unk4)->unk10;
                        if ((((UnkNode2*)temp_s6_2)->unk4 & 3) == 1) {
                            a0->unk10 = (s32) (-((temp_s0_2 + func_8005DFAC(temp_a0_3, &probe.x, 0, 0)) << 8) - a0->unk4);
                        } else {
                            a0->unk10 = (s32) (a0->unk10 + (((UnkNode1*)temp_a0_3)->unk28 - (temp_s0_2 << 8)));
                        }
                        if (temp_s2_2 != a0->unk10) {
                            sp50 |= 0x30;
                        } else if ((((UnkNode1*)temp_a0_3)->unk24 != 0) || (((UnkNode1*)temp_a0_3)->unk30 != 0)) {
                            sp50 |= 0x10;
                        }
                    }
                }
            }
        }
    }
    if ((a0->unkC == 0) && (a0->unk10 == 0) && (a0->unk14 == 0)) {
        var_v0 = 0;
        if ((sp2C != NULL) && (var_v0 = 0, (a0->unk4 != 0))) {
            if ((a0->unk20 & 1) && (temp_a0_5 = a0->unk1C, (temp_a0_5 != (void* )-2))) {
                var_v0 = 0;
                if (temp_a0_5 != NULL) {
                    temp_s6_3 = ((UnkNode1*)temp_a0_5)->unk4;
                    temp_v1_6 = ((UnkNode2*)temp_s6_3)->unk4 & 3;
                    switch (temp_v1_6) {            /* switch 2; irregular */
                    case 0:                         /* switch 2 */
                        var_v0_12 = ((UnkNode1*)temp_a0_5)->unk38 + (((UnkNode2*)temp_s6_3)->unk10 << 8);
                        a0->unk18 = (s32) -var_v0_12;
                        return 0;
                    case 1:                         /* switch 2 */
                        var_v0_13 = a0->unk0;
                        if (var_v0_13 < 0) {
                            var_v0_13 += 0xFF;
                        }
                        probe.x = var_v0_13 >> 8;
                        var_v0_14 = a0->unk8;
                        if (var_v0_14 < 0) {
                            var_v0_14 += 0xFF;
                        }
                        probe.y = var_v0_14 >> 8;
                        temp_s3_2 = sp28->unkC;
                        var_s0_3 = func_8005DFAC(temp_a0_5, &probe.x, 0, 0);
                        if ((temp_s3_2 != NULL) && (temp_s3_2 != temp_a0_5)) {
                            var_s0_3 += func_8005DFAC(temp_s3_2, &probe.x, 0, 0) - ((UnkNode2*)((UnkNode1*)temp_s3_2)->unk4)->unk10;
                        }
                        var_v0_12 = var_s0_3 << 8;
                        a0->unk18 = (s32) -var_v0_12;
                        return 0;
                    default:                        /* switch 2 */
                        return 0;
                    }
                } else {
                    /* Duplicate return node #441. Try simplifying control flow for better match */
                    return var_v0;
                }
            }
        } else {
            /* Duplicate return node #441. Try simplifying control flow for better match */
            return var_v0;
        }
    }
    {
        probe.m = a0;
        probe.w = sp30;
        probe.h = sp38;
        var_v0_15 = a0->unkC;
        temp_v1_7 = a0->unk24;
        if (var_v0_15 < 0) {
            var_v0_15 = -var_v0_15;
        }
        temp_v0_2 = var_v0_15 >> 8;
        sp54 = temp_v0_2;
        var_t9 = 1;
        if (temp_v0_2 >= temp_v1_7) {
            var_t9 = (sp54 / temp_v1_7) + 1;
        }
        var_v0_16 = a0->unk14;
        temp_v1_8 = (s16) a0->unk28;
        if (var_v0_16 < 0) {
            var_v0_16 = -var_v0_16;
        }
        temp_v0_3 = var_v0_16 >> 8;
        sp54 = temp_v0_3;
        if (temp_v0_3 >= temp_v1_8) {
            temp_v0_4 = (sp54 / temp_v1_8) + 1;
            sp54 = temp_v0_4;
            if (var_t9 < temp_v0_4) {
                var_t9 = sp54;
            }
        }
        if (var_t9 == 1) {
            var_v0_17 = a0->unk0 + a0->unkC;
            if (var_v0_17 < 0) {
                var_v0_17 += 0xFF;
            }
            probe.x = var_v0_17 >> 8;
            var_v0_18 = a0->unk8 + a0->unk14;
            if (var_v0_18 < 0) {
                var_v0_18 += 0xFF;
            }
            probe.y = var_v0_18 >> 8;
            func_8005DA7C((void**)&probe, sp2C, &sp20, &sp24);
        } else {
            sp54 = 0;
            do {
                temp_t5_3 = sp54 + 1;
                sp54 = temp_t5_3;
                var_v0_19 = a0->unk0 + ((s32) (a0->unkC * temp_t5_3) / var_t9);
                if (var_v0_19 < 0) {
                    var_v0_19 += 0xFF;
                }
                probe.x = var_v0_19 >> 8;
                var_v0_20 = a0->unk8 + ((s32) (a0->unk14 * sp54) / var_t9);
                if (var_v0_20 < 0) {
                    var_v0_20 += 0xFF;
                }
                probe.y = var_v0_20 >> 8;
                func_8005DA7C((void**)&probe, sp2C, &sp20, &sp24);
            } while ((var_t9 != sp54) && (sp20 == 0));
        }
        var_t8 = 0;
        if (sp20 == 0) {
            temp_a2 = sp28->unk0;
            if ((((UnkNode3*)temp_a2)->unk2C & 2) && ((temp_a0_6 = (u16) a0->unk24, temp_a1_2 = (u16) a0->unk28, temp_a3 = (u16) probe.x - ((s32) ((s16) temp_a0_6 + ((u32) (temp_a0_6 << 0x10) >> 0x1F)) >> 1), temp_v0_5 = (u16) probe.y - ((s32) ((s16) temp_a1_2 + ((u32) (temp_a1_2 << 0x10) >> 0x1F)) >> 1), (temp_v0_5 & 0x8000)) || ((s16) (temp_v0_5 + temp_a1_2) >= ((UnkNode3*)temp_a2)->unk32) || (temp_a3 & 0x8000) || ((s16) (temp_a3 + temp_a0_6) >= ((UnkNode3*)temp_a2)->unk30))) {
                var_t8 = 1;
            }
        }
        if ((sp20 == 0) && (var_t8 == 0)) {
            var_v1 = a0->unk8 + a0->unk14;
            a0->unk0 = (s32) (a0->unk0 + a0->unkC);
            a0->unk8 = var_v1;
        } else {
        temp_a1_3 = a0->unkC;
        if (temp_a1_3 == 0) {
            sp58 = 0xC00;
            if (a0->unk14 >= 0) {
                sp58 = 0x400;
            }
        } else {
            sp58 = ratan2(a0->unk14, temp_a1_3) & 0xFFF;
        }
        temp_v0_6 = a0->unkC;
        var_v1_2 = a0->unk14;
        var_t9 = temp_v0_6;
        if (temp_v0_6 < 0) {
            var_t9 = -var_t9;
        }
        if (var_v1_2 < 0) {
            var_v1_2 = -var_v1_2;
        }
        sp54 = var_v1_2;
        if (var_t9 >= var_v1_2) {
            var_t9 = var_t9 >> 8;
        } else {
            var_t9 = sp54 >> 8;
        }
        var_s5 = -2;
        if (var_t9 == 0) {
            var_t9 = 1;
        }
        sp54 = var_t9 - 1;
        if (var_t9 != 0) {
            do {
            temp_v0_7 = (u16) a0->unk24;
            var_v0_21 = a0->unk0 + ((s32) (a0->unkC * (var_t9 - sp54)) / var_t9);
            if (var_v0_21 < 0) {
                var_v0_21 += 0xFF;
            }
            temp_s1 = (var_v0_21 >> 8) - ((s32) ((s16) temp_v0_7 + ((u32) (temp_v0_7 << 0x10) >> 0x1F)) >> 1);
            temp_v0_8 = (u16) a0->unk28;
            temp_s2_3 = temp_s1 + (u16) a0->unk24;
            var_v0_22 = a0->unk8 + ((s32) (a0->unk14 * (var_t9 - sp54)) / var_t9);
            if (var_v0_22 < 0) {
                var_v0_22 += 0xFF;
            }
            temp_s0_3 = (var_v0_22 >> 8) - ((s32) ((s16) temp_v0_8 + ((u32) (temp_v0_8 << 0x10) >> 0x1F)) >> 1);
            temp_a0_7 = sp28->unk0;
            temp_v1_9 = temp_s0_3 + (u16) a0->unk28;
            if (((UnkNode3*)temp_a0_7)->unk2C & 2) {
                if ((temp_s0_3 < 0) || (temp_v1_9 >= ((UnkNode3*)temp_a0_7)->unk32)) {
                    var_s5 = func_8005E1A8(NULL, 0x7F, sp58, var_s5);
                }
                if ((temp_s1 < 0) || (temp_s2_3 >= ((UnkNode3*)sp28->unk0)->unk30)) {
                    var_s5 = func_8005E1A8(NULL, 0x7E, sp58, var_s5);
                }
            }
            sp64 = (void** )0x801E1000;
            sp68 = (void** )0x801E1100;
            var_t7 = sp20 - 1;
            sp24 = 0;
            if (var_t7 != -1) {
                e_x_min = temp_s1;
                sp74 = temp_s0_3;
                sp70 = temp_v1_9;
                sp78 = temp_v1_9 - 1;
                sp6C = temp_s2_3;
                sp7C = temp_s2_3 - 1;
                do {
                    temp_fp = *sp64;
                    sp64 += 1;
                    temp_a0_8 = (s32) ((UnkNode1*)sp2C)->unk40 >> 8;
                    temp_a2_2 = (s32) (((UnkNode1*)sp2C)->unk34 << 8) >> 0x10;
                    temp_s6_4 = ((UnkNode1*)temp_fp)->unk4;
                    if (((((UnkNode1*)temp_fp)->unk1C + temp_a2_2) < sp6C) && (((((UnkNode1*)temp_fp)->unk1E + temp_a2_2) >= e_x_min))) {
                        temp_a1_4 = ((UnkNode1*)temp_fp)->unk22 + (s16) temp_a0_8;
                        if (temp_a1_4 < sp70) {
                            temp_v1_10 = ((UnkNode1*)temp_fp)->unk20 + (s16) temp_a0_8;
                            if (temp_v1_10 >= sp74) {
                                var_s0_4 = temp_v1_10;
                                var_t1 = temp_a1_4;
                                if (sp78 < temp_v1_10) {
                                    var_s0_4 = sp78;
                                }
                                if (var_t1 < sp74) {
                                    var_t1 = sp74;
                                }
                                temp_a0_9 = ((u8*)temp_s6_4)[6];
                                temp_lo = (var_t1 - temp_a1_4) * temp_a0_9;
                                temp_lo_2 = ((var_s0_4 - var_t1) + 1) * temp_a0_9;
                                var_t8 = 0;
                                var_s1_2 = (u8*)((UnkNode1*)temp_fp)->unk10 + (temp_lo * 4);
                                var_s4 = (s8*)((UnkNode1*)temp_fp)->unk14 + (temp_lo * 2);
                                if (((UnkNode2*)temp_s6_4)->unk4 & 8) {
                                    var_s0_5 = temp_lo_2 - 1;
                                    var_s3 = var_s4 + 1;
                                    if (var_s0_5 != -1) {
                                        var_s2_2 = var_s1_2 + 2;
                                        do {
                                            temp_v1_11 = *(s16*)var_s1_2;
                                            if ((temp_v1_11 < sp6C) && (*(s16*)var_s2_2 >= e_x_min)) {
                                                if ((e_x_min < temp_v1_11) && (*var_s4 >= 0)) {
                                                    var_s5 = func_8005E1A8(temp_s6_4, (u8) *var_s4 & 0x7F, sp58, var_s5);
                                                    var_t8 = 2;
                                                }
                                                if ((*(s16*)var_s2_2 < sp7C) && (*var_s3 >= 0)) {
                                                    var_s5 = func_8005E1A8(temp_s6_4, (u8) *var_s3 & 0x7F, sp58, var_s5);
                                                    var_t8 = 2;
                                                }
                                                if ((*var_s4 >= 0) && (*var_s3 >= 0) && (((u8) *var_s4 == 0x7F) || ((u8) *var_s3 == 0x7F)) && (*(s16*)var_s1_2 < e_x_min) && (*(s16*)var_s2_2 >= sp6C)) {
                                                    var_s5 = func_8005E1A8(temp_s6_4, 0x7F, sp58, var_s5);
                                                    var_t8 = 2;
                                                }
                                            }
                                            var_s2_2 += 4;
                                            var_s1_2 += 4;
                                            var_s3 += 2;
                                            var_s0_5 -= 1;
                                            var_s4 += 2;
                                        } while (var_s0_5 != -1);
                                    }
                                } else if (temp_a2_2 != 0) {
                                    var_s0_6 = temp_lo_2 - 1;
                                    if (var_s0_6 != -1) {
                                        var_s2_3 = var_s1_2 + 2;
                                        do {
                                            temp_v1_12 = *(s16*)var_s1_2 + temp_a2_2;
                                            if ((temp_v1_12 < sp6C) && ((*(s16*)var_s2_3 + temp_a2_2) >= e_x_min)) {
                                                var_t8 = 1;
                                                if (e_x_min < temp_v1_12) {
                                                    var_s5 = func_8005E1A8(temp_s6_4, var_s4[0] & 0x7F, sp58, var_s5);
                                                    var_t8 = 1;
                                                }
                                                if ((*(s16*)var_s2_3 + temp_a2_2) < sp7C) {
                                                    var_s5 = func_8005E1A8(temp_s6_4, var_s4[1] & 0x7F, sp58, var_s5);
                                                    var_t8 = 1;
                                                }
                                                if (((*(s16*)var_s1_2 + temp_a2_2) < e_x_min) && ((*(s16*)var_s2_3 + temp_a2_2) >= sp6C)) {
                                                    var_s5 = func_8005E1A8(temp_s6_4, 0x7F, sp58, var_s5);
                                                }
                                            }
                                            var_s2_3 += 4;
                                            var_s1_2 += 4;
                                            var_s0_6 -= 1;
                                            var_s4 += 2;
                                        } while (var_s0_6 != -1);
                                    }
                                } else {
                                    var_s0_7 = temp_lo_2 - 1;
                                    if (var_s0_7 != -1) {
                                        var_s2_4 = var_s1_2 + 2;
                                        do {
                                            temp_v1_13 = *(s16*)var_s1_2;
                                            if ((temp_v1_13 < sp6C) && (*(s16*)var_s2_4 >= e_x_min)) {
                                                var_t8 = 1;
                                                if (e_x_min < temp_v1_13) {
                                                    var_s5 = func_8005E1A8(temp_s6_4, var_s4[0] & 0x7F, sp58, var_s5);
                                                    var_t8 = 1;
                                                }
                                                if (*(s16*)var_s2_4 < sp7C) {
                                                    var_s5 = func_8005E1A8(temp_s6_4, var_s4[1] & 0x7F, sp58, var_s5);
                                                    var_t8 = 1;
                                                }
                                                if ((*(s16*)var_s1_2 < e_x_min) && (*(s16*)var_s2_4 >= sp6C)) {
                                                    var_s5 = func_8005E1A8(temp_s6_4, 0x7F, sp58, var_s5);
                                                }
                                            }
                                            var_s2_4 += 4;
                                            var_s1_2 += 4;
                                            var_s0_7 -= 1;
                                            var_s4 += 2;
                                        } while (var_s0_7 != -1);
                                    }
                                }
                                if (var_t8 != 0) {
                                    *sp68 = temp_fp;
                                    sp68 += 1;
                                    sp24 += 1;
                                }
                            }
                        }
                    }
                    var_t7 -= 1;
                } while (var_t7 != -1);
            }
            if (var_s5 != -2) {
                break;
            }
            temp_t6_2 = sp54 - 1;
            sp54 = temp_t6_2;
            } while (temp_t6_2 != -1);
        }
        temp_a0_10 = a0->unkC;
        temp_v0_9 = (var_t9 - 1) - sp54;
        sp5C = (s32) (temp_a0_10 * temp_v0_9) / var_t9;
        temp_v1_14 = a0->unk14;
        temp_lo_3 = (s32) (temp_v1_14 * temp_v0_9) / var_t9;
        sp60 = temp_lo_3;
        if ((var_s5 >= 0) && (sp5C == 0) && (temp_lo_3 == 0)) {
            temp_s0_4 = SquareRoot0((temp_a0_10 * temp_a0_10) + (temp_v1_14 * temp_v1_14));
            if ((temp_s0_4 * rcos(var_s5)) >= 0) {
                sp5C = (s32) (temp_s0_4 * rcos(var_s5)) >> 0xC;
            } else {
                sp5C = (s32) ((temp_s0_4 * rcos(var_s5)) + 0xFFF) >> 0xC;
            }
            if ((temp_s0_4 * rsin(var_s5)) >= 0) {
                sp60 = (s32) (temp_s0_4 * rsin(var_s5)) >> 0xC;
            } else {
                sp60 = (s32) ((temp_s0_4 * rsin(var_s5)) + 0xFFF) >> 0xC;
            }
            sp20 = 1;
            sp80 = (s16) (u16) a0->unk24 / 2;
            do {
            temp_v0_10 = a0->unk0 + sp5C;
            if (temp_v0_10 >= 0) {
                var_s1_3 = (temp_v0_10 >> 8) - sp80;
            } else {
                var_s1_3 = ((s32) (temp_v0_10 + 0xFF) >> 8) - sp80;
            }
            temp_v0_11 = (u16) a0->unk28;
            temp_s2_4 = var_s1_3 + (u16) a0->unk24;
            var_v0_24 = a0->unk8 + sp60;
            if (var_v0_24 < 0) {
                var_v0_24 += 0xFF;
            }
            temp_s0_5 = (var_v0_24 >> 8) - ((s32) ((s16) temp_v0_11 + ((u32) (temp_v0_11 << 0x10) >> 0x1F)) >> 1);
            var_t8 = 0;
            var_t0 = 0;
            var_a3_2 = 0;
            sp68 = (void** )0x801E1100;
            temp_a0_11 = sp28->unk0;
            temp_v1_15 = temp_s0_5 + (u16) a0->unk28;
            if (((UnkNode3*)temp_a0_11)->unk2C & 2) {
                if (temp_s0_5 < 0) {
                    var_a3_2 = -temp_s0_5;
                    var_t8 = 1;
                } else {
                    temp_a0_12 = ((UnkNode3*)temp_a0_11)->unk32;
                    if (temp_v1_15 >= temp_a0_12) {
                        var_a3_2 = (temp_a0_12 - temp_v1_15) - 1;
                        var_t8 = 1;
                    }
                }
                if (var_s1_3 < 0) {
                    var_t0 = -var_s1_3;
                    var_t8 = 1;
                } else {
                    temp_a0_13 = ((UnkNode3*)sp28->unk0)->unk30;
                    if (temp_s2_4 >= temp_a0_13) {
                        var_t0 = (temp_a0_13 - temp_s2_4) - 1;
                        var_t8 = 1;
                    }
                }
            }
            var_t9 = sp24 - 1;
            if (var_t9 != -1) {
                f_x_max = temp_s2_4;
                f_x_min = var_s1_3;
                f_y_min = temp_s0_5;
                f_y_max = temp_v1_15;
                sp8C = f_y_max - 1;
                temp_v1_16 = (s32) ((UnkNode1*)sp2C)->unk34 >> 8;
                sp40 = (u16) temp_v1_16;
                sp84 = (s32) (s16) temp_v1_16;
                sp88 = (s32) (((UnkNode1*)sp2C)->unk40 << 8) >> 0x10;
                do {
                    temp_fp_2 = *sp68;
                    sp68 += 1;
                    temp_s6_5 = ((UnkNode1*)temp_fp_2)->unk4;
                    if (((((UnkNode1*)temp_fp_2)->unk1C + sp84) < f_x_max) && (((((UnkNode1*)temp_fp_2)->unk1E + sp84) >= f_x_min))) {
                        temp_a0_14 = ((UnkNode1*)temp_fp_2)->unk22 + sp88;
                        if (temp_a0_14 < f_y_max) {
                            temp_v1_17 = ((UnkNode1*)temp_fp_2)->unk20 + sp88;
                            if (temp_v1_17 >= f_y_min) {
                                var_s0_8 = temp_v1_17;
                                var_t1_2 = temp_a0_14;
                                if (sp8C < temp_v1_17) {
                                    var_s0_8 = sp8C;
                                }
                                if (var_t1_2 < f_y_min) {
                                    var_t1_2 = f_y_min;
                                }
                                temp_lo_4 = (var_t1_2 - temp_a0_14) * ((u8*)temp_s6_5)[6];
                                var_s0_9 = var_s0_8 - var_t1_2;
                                var_s1_4 = (u8*)((UnkNode1*)temp_fp_2)->unk10 + (temp_lo_4 * 4);
                                var_s4_2 = (u8*)((UnkNode1*)temp_fp_2)->unk14 + (temp_lo_4 * 2);
                                if (var_s0_9 != -1) {
                                    temp_s7 = f_x_max - 1;
                                    do {
                                        var_t7 = ((u8*)temp_s6_5)[6] - 1;
                                        if (var_t7 != -1) {
                                            temp_s5 = var_t1_2 + 1;
                                            var_t3 = var_s1_4 + 2;
                                            spAC = (s32) (s16) sp40;
                                            spA8 = var_s4_2 + 1;
                                            do {
                                                temp_v1_18 = *(s16*)var_s1_4;
                                                if (temp_v1_18 < f_x_max) {
                                                    temp_a1_5 = *(s16*)var_t3;
                                                    if (temp_a1_5 >= f_x_min) {
                                                        var_a2 = 0;
                                                        if (((UnkNode2*)temp_s6_5)->unk4 & 8) {
                                                            var_a0 = 0;
                                                            if ((f_x_min < temp_v1_18) && !(*var_s4_2 & 0x80)) {
                                                                var_a2 = 1;
                                                                if (temp_a1_5 >= f_x_max) {
                                                                    var_a0 = temp_v1_18 - f_x_max;
                                                                }
                                                            } else {
                                                                temp_v1_19 = *(s16*)var_t3;
                                                                if ((temp_v1_19 < temp_s7) && !(*spA8 & 0x80)) {
                                                                    if (*(s16*)var_s1_4 < f_x_min) {
                                                                        var_a0 = (temp_v1_19 - f_x_min) + 1;
                                                                    }
                                                                    var_a2 = 1;
                                                                }
                                                            }
                                                            if (!(*var_s4_2 & 0x80) && !(*spA8 & 0x80)) {
                                                                var_a2 = 1;
                                                            }
                                                        } else {
                                                            var_a2 = 1;
                                                            temp_v1_20 = temp_v1_18 + spAC;
                                                            var_a0 = 0;
                                                            if ((f_x_min < temp_v1_20) && ((temp_a1_5 + spAC) >= f_x_max)) {
                                                                var_a0 = temp_v1_20 - f_x_max;
                                                            } else if ((*(s16*)var_s1_4 + spAC) < f_x_min) {
                                                                temp_v1_21 = *(s16*)var_t3 + spAC;
                                                                if (temp_v1_21 < temp_s7) {
                                                                    var_a0 = (temp_v1_21 - f_x_min) + 1;
                                                                }
                                                            }
                                                        }
                                                        temp_v1_22 = var_t1_2 - f_y_min;
                                                        if (var_a2 != 0) {
                                                            var_t8 = 1;
                                                            if ((f_y_max - temp_s5) >= temp_v1_22) {
                                                                var_a1 = temp_v1_22 + 1;
                                                            } else {
                                                                var_a1 = (temp_s5 - f_y_max) - 1;
                                                            }
                                                            if ((var_t0 != 0) || (var_a3_2 != 0)) {
                                                                if ((var_t0 != 0x8000) && (var_a0 != 0)) {
                                                                    if (var_a0 > 0) {
                                                                        var_v0_26 = var_t0 < var_a0;
                                                                        if (var_t0 >= 0) {
                                                                            if (var_v0_26 != 0) {
                                                                                var_t0 = var_a0;
                                                                            }
                                                                        } else {
                                                                            var_t0 = 0x8000;
                                                                        }
                                                                    } else {
                                                                        var_v0_26 = var_a0 < var_t0;
                                                                        if (var_t0 <= 0) {
                                                                            if (var_v0_26 != 0) {
                                                                                var_t0 = var_a0;
                                                                            }
                                                                        } else {
                                                                            var_t0 = 0x8000;
                                                                        }
                                                                    }
                                                                }
                                                                if (var_a3_2 != 0x8000) {
                                                                    if (var_a1 > 0) {
                                                                        if (var_a3_2 >= 0) {
                                                                            if (var_a3_2 < var_a1) {
                                                                                var_a3_2 = var_a1;
                                                                            }
                                                                        } else {
                                                                            var_a3_2 = 0x8000;
                                                                        }
                                                                    } else if (var_a3_2 <= 0) {
                                                                        if (var_a1 < var_a3_2) {
                                                                            var_a3_2 = var_a1;
                                                                        }
                                                                    } else {
                                                                        var_a3_2 = 0x8000;
                                                                    }
                                                                }
                                                            } else {
                                                                var_t0 = var_a0;
                                                                var_a3_2 = var_a1;
                                                            }
                                                        }
                                                    }
                                                }
                                                var_t3 += 4;
                                                var_s1_4 += 4;
                                                var_s4_2 += 2;
                                                var_t7 -= 1;
                                                spA8 += 2;
                                            } while (var_t7 != -1);
                                        }
                                        var_s0_9 -= 1;
                                        var_t1_2 += 1;
                                    } while (var_s0_9 != -1);
                                }
                            }
                        }
                    }
                    var_t9 -= 1;
                } while (var_t9 != -1);
            }
            if (var_t8 == 0) {
                break;
            }
            temp_a0_15 = var_t0 << 8;
            if (sp20 == 1) {
                temp_a1_6 = var_a3_2 << 8;
                if ((var_t0 == 0) || (var_t0 == 0x8000)) {
                    if (var_a3_2 != 0x8000) {
                        sp60 += temp_a1_6;
                    }
                    break;
                } else if ((var_a3_2 == 0) || (var_a3_2 == 0x8000)) {
                    sp5C += temp_a0_15;
                    break;
                } else {
                    var_v1_3 = temp_a0_15;
                    if (var_v1_3 < 0) {
                        var_v1_3 = -var_v1_3;
                    }
                    var_v0_27 = temp_a1_6;
                    if (var_v0_27 < 0) {
                        var_v0_27 = -var_v0_27;
                    }
                    if (var_v0_27 < var_v1_3) {
                        sp48 = temp_a0_15;
                        sp4C = 0;
                        sp60 += temp_a1_6;
                    } else {
                        sp4C = temp_a1_6;
                        sp48 = 0;
                        sp5C += temp_a0_15;
                    }
                }
            } else {
                sp5C += sp48;
                sp60 += sp4C;
                break;
            }
            temp_v0_13 = sp20 - 1;
            sp20 = temp_v0_13;
            } while (temp_v0_13 != -1);
            sp50 |= 2;
        } else {
            sp50 |= 1;
        }
        if ((sp5C != 0) || (sp60 != 0)) {
            var_v0_28 = a0->unk0 + sp5C;
            if (var_v0_28 < 0) {
                var_v0_28 += 0xFF;
            }
            probe.x = var_v0_28 >> 8;
            var_v0_29 = a0->unk8 + sp60;
            if (var_v0_29 < 0) {
                var_v0_29 += 0xFF;
            }
            probe.y = var_v0_29 >> 8;
            func_8005DA7C((void**)&probe, sp2C, &sp20, &sp24);
            var_t8 = 0;
            if (sp20 == 0) {
                temp_a2_3 = sp28->unk0;
                if ((((UnkNode3*)temp_a2_3)->unk2C & 2) && ((temp_a0_16 = (u16) a0->unk24, temp_a1_7 = (u16) a0->unk28, temp_a3_2 = (u16) probe.x - ((s32) ((s16) temp_a0_16 + ((u32) (temp_a0_16 << 0x10) >> 0x1F)) >> 1), temp_v0_14 = (u16) probe.y - ((s32) ((s16) temp_a1_7 + ((u32) (temp_a1_7 << 0x10) >> 0x1F)) >> 1), (temp_v0_14 & 0x8000)) || ((s16) (temp_v0_14 + temp_a1_7) >= ((UnkNode3*)temp_a2_3)->unk32) || (temp_a3_2 & 0x8000) || ((s16) (temp_a3_2 + temp_a0_16) >= ((UnkNode3*)temp_a2_3)->unk30))) {
                    var_t8 = 1;
                }
            }
            if ((sp20 == 0) && (var_t8 == 0)) {
                a0->unk0 = (s32) (a0->unk0 + sp5C);
                var_v1 = a0->unk8 + sp60;
                a0->unk8 = var_v1;
            } else {
                var_v0_30 = a0->unk0;
                if (var_v0_30 < 0) {
                    var_v0_30 += 0xFF;
                }
                probe.x = var_v0_30 >> 8;
                var_v0_31 = a0->unk8;
                if (var_v0_31 < 0) {
                    var_v0_31 += 0xFF;
                }
                probe.y = var_v0_31 >> 8;
                func_8005DA7C((void**)&probe, sp2C, &sp20, &sp24);
                sp50 |= 3;
            }
        } else {
            var_v0_32 = a0->unk0;
            if (var_v0_32 < 0) {
                var_v0_32 += 0xFF;
            }
            probe.x = var_v0_32 >> 8;
            var_v0_33 = a0->unk8;
            if (var_v0_33 < 0) {
                var_v0_33 += 0xFF;
            }
            probe.y = var_v0_33 >> 8;
            func_8005DA7C((void**)&probe, sp2C, &sp20, &sp24);
            sp50 |= 3;
        }
        }
        var_s7 = 0xFFFFFF;
        var_s2 = 0;
        var_s4_3 = 0;
        var_a3 = 0;
        var_v0_34 = a0->unk0;
        var_fp = NULL;
        a0->unk1C = NULL;
        if (var_v0_34 < 0) {
            var_v0_34 += 0xFF;
        }
        probe.x = var_v0_34 >> 8;
        var_v0_35 = a0->unk8;
        if (var_v0_35 < 0) {
            var_v0_35 += 0xFF;
        }
        probe.y = var_v0_35 >> 8;
        ux = (u16) probe.x;
        uy = (u16) probe.y;
        sp68 = (void** )0x801E1100;
        temp_s3_3 = sp28->unkC;
        temp_v0_15 = sp24 - 1;
        sp24 = temp_v0_15;
        if (temp_v0_15 != -1) {
            var_t0_2 = (s16) uy;
            var_t1_3 = (s16) sp38;
            do {
                temp_t5_4 = *sp68;
                sp68 += 1;
                sp2C = temp_t5_4;
                temp_s6_6 = ((UnkNode1*)temp_t5_4)->unk4;
                temp_a1_8 = (s32) ((UnkNode1*)temp_t5_4)->unk34 >> 8;
                temp_a0_17 = (s32) (((UnkNode1*)temp_t5_4)->unk40 << 8) >> 0x10;
                temp_v1_24 = ((UnkNode1*)temp_t5_4)->unk22 + temp_a0_17;
                var_t8 = 0;
                if ((var_t0_2 >= temp_v1_24) && (((((UnkNode1*)temp_t5_4)->unk20 + temp_a0_17) >= var_t0_2))) {
                    temp_a0_18 = ((u8*)temp_s6_6)[6];
                    var_s1_5 = (u8*)((UnkNode1*)temp_t5_4)->unk10 + ((var_t0_2 - temp_v1_24) * temp_a0_18 * 4);
                    if ((s16) temp_a1_8 != 0) {
                        var_s0_10 = temp_a0_18 - 1;
                        if (temp_a0_18 != 0) {
                            do {
                                if (((s16) ux < (((UnkS16*)var_s1_5)->unk0 + (s16) temp_a1_8)) || ((((UnkS16*)var_s1_5)->unk2 + (s16) temp_a1_8) < (s16) ux)) {
                                    var_s1_5 += 4;
                                } else {
                                    var_t8 = 1;
                                    break;
                                }
                            } while (--var_s0_10 != -1);
                        }
                    } else {
                        var_s0_11 = temp_a0_18 - 1;
                        if (temp_a0_18 != 0) {
                            do {
                                if (((s16) ux < ((UnkS16*)var_s1_5)->unk0) || (((UnkS16*)var_s1_5)->unk2 < (s16) ux)) {
                                    var_s1_5 += 4;
                                } else {
                                    var_t8 = 1;
                                    break;
                                }
                            } while (--var_s0_11 != -1);
                        }
                    }
                }
                temp_a1_9 = ((UnkNode1*)sp2C)->unk38;
                temp_a0_19 = ((UnkNode2*)temp_s6_6)->unk4 & 3;
                temp_v1_25 = temp_a1_9 >> 8;
                switch (temp_a0_19) {               /* switch 3; irregular */
                case 0:                             /* switch 3 */
                    var_v1_4 = ((UnkNode2*)temp_s6_6)->unk14 + (s16) temp_v1_25;
                    var_v0_36 = var_v1_4 < var_s7;
                    if (var_v1_4 < var_t1_3) {
                        if (var_t8 != 0) {
                            temp_v0_16 = -(temp_a1_9 + (((UnkNode2*)temp_s6_6)->unk10 << 8));
                            if (temp_v0_16 < a0->unk18) {
                                a0->unk18 = temp_v0_16;
                                var_fp = sp2C;
                            }
                        }
                        if (var_a3 != 0) {
                            temp_v0_17 = ((UnkNode2*)temp_s6_6)->unk10 + (s16) temp_v1_25;
                            if ((var_s2 + 0x14) < temp_v0_17) {
                                var_s2 = temp_v0_17;
                                var_s4_3 = ((UnkNode1*)sp2C)->unk38 + (((UnkNode2*)temp_s6_6)->unk10 << 8);
                                if (var_t8 != 0) {
                                    a0->unk1C = sp2C;
                                }
                            }
                        } else {
                            temp_v1_26 = ((UnkNode2*)temp_s6_6)->unk10 + (s16) temp_v1_25;
                            if (temp_v1_26 >= var_s2) {
                                var_s2 = temp_v1_26;
                                var_s4_3 = ((UnkNode1*)sp2C)->unk38 + (((UnkNode2*)temp_s6_6)->unk10 << 8);
                                if (var_t8 != 0) {
                                    a0->unk1C = sp2C;
                                }
                            }
                        }
                    } else {
                        if (var_v0_36 != 0) {
                            var_s7 = var_v1_4;
                        }
                    }
                    break;
                case 1:                             /* switch 3 */
                    var_v1_4 = ((UnkNode2*)temp_s6_6)->unk14 + (s16) temp_v1_25;
                    if (var_v1_4 < var_t1_3) {
                        temp_s0_6 = func_8005DFAC(sp2C, &probe.x, temp_v1_25, var_a3);
                        if (var_t8 != 0) {
                            temp_v1_27 = -(temp_s0_6 << 8);
                            if (temp_v1_27 < a0->unk18) {
                                a0->unk18 = temp_v1_27;
                                var_fp = sp2C;
                            }
                        }
                        if (var_a3 != 0) {
                            var_v0_37 = var_s2 < temp_s0_6;
                            if (temp_s3_3 != NULL) {
                                if (temp_s3_3 == sp2C) {
                                    temp_v0_18 = temp_s0_6 - ((UnkNode2*)((UnkNode1*)sp2C)->unk4)->unk10;
                                    var_s2 += temp_v0_18;
                                    var_s4_3 += temp_v0_18 << 8;
                                } else {
                                    var_s2 = (temp_s0_6 + var_s2) - ((UnkNode2*)((UnkNode1*)temp_s3_3)->unk4)->unk10;
                                    var_s4_3 = var_s2 << 8;
                                    if (var_t8 != 0) {
                                        a0->unk1C = sp2C;
                                    }
                                }
                                a0->unk18 = (s32) -(var_s2 << 8);
                                var_fp = sp2C;
                                var_a3 = 1;
                            } else {
                                var_a3 = 1;
                                if (var_v0_37 != 0) {
                                    var_s2 = temp_s0_6;
                                    var_s4_3 = var_s2 << 8;
                                    if (var_t8 != 0) {
                                        a0->unk1C = sp2C;
                                    }
                                }
                            }
                        } else {
                            var_v0_37 = var_s2 < (temp_s0_6 + 0x14);
                            var_a3 = 1;
                            if (var_v0_37 != 0) {
                                var_s2 = temp_s0_6;
                                var_s4_3 = var_s2 << 8;
                                if (var_t8 != 0) {
                                    a0->unk1C = sp2C;
                                }
                            }
                        }
                    } else {
                        var_v0_36 = var_v1_4 < var_s7;
                        if (var_v0_36 != 0) {
                            var_s7 = var_v1_4;
                        }
                    }
                    break;
                }
                temp_v1_23 = sp24 - 1;
                sp24 = temp_v1_23;
            } while (temp_v1_23 != -1);
        }
        if (a0->unk28 & 0x30000) {
            temp_a0_20 = a0->unk26;
            if (var_s7 < ((s16) sp30 + temp_a0_20)) {
                a0->unk4 = (s32) -((var_s7 - temp_a0_20) << 8);
                var_t5 = sp50 | 0x40;
                sp50 = var_t5;
            } else if (var_s2 >= (s16) sp30) {
                a0->unk4 = (s32) -var_s4_3;
                var_t5 = sp50 | 0x80;
                sp50 = var_t5;
            } else {
                a0->unk4 = (s32) (a0->unk4 + a0->unk10);
            }
        } else if (((s16) sp30 != var_s2) || (var_s4_3 != 0)) {
            a0->unk4 = (s32) -var_s4_3;
            var_t5 = sp50 | 0x20;
            sp50 = var_t5;
        }
        if (var_fp == a0->unk1C) {
            a0->unk20 = (s32) (a0->unk20 | 1);
        } else {
            a0->unk20 = (s32) (a0->unk20 & 0xFFFE);
        }
        var_v0 = sp50;
        return var_v0;
    }
}