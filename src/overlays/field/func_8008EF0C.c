#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))

extern u8 D_800EB074;
extern u8 D_800EB114;
extern u8 D_800FD818;
extern u8 D_800FDF58;
extern u8 D_80105880;
extern u8 D_80105AE0;
extern u8 D_8010A038;
extern u8 D_8010A090;
extern s32 D_8010AE54;
extern u8 g_field_actor_slots;
extern u8 g_field_resource_entries;

void field_stop_actor_animations_for_object(void *record, s32 force);

typedef struct Func8008EF0CScratch {
    s32 sp18;
    u8 pad1C[0x2C];
    u16 sp48;
    u8 pad4A[2];
    u16 sp4C;
    s16 sp4E;
    s32 sp50;
    s32 sp54;
    s32 sp58;
    u8 pad5C[4];
    s32 sp60;
    s32 sp64;
    u8 tail68[8];
} Func8008EF0CScratch;

typedef struct Func8008Slot {
    u8 pad000[0x16F];
    u8 unk16F;
    u8 pad170[0x172 - 0x170];
    u16 unk172;
    u32 unk174;
    union {
        s32 word;
        struct { u8 b178, b179, b17A, b17B; } b;
    } u178;
    u8 pad17C[4];
    u8 unk180[0x23C - 0x180];
} Func8008Slot;

typedef struct Func8008FDEntry {
    u8 unk0, unk1, unk2, unk3;
    u8 pad004[0x257 - 4];
    u8 unk257;
    u8 pad258[0x268 - 0x258];
} Func8008FDEntry;

#define SLOT23C(i) (((Func8008Slot *)&D_80105AE0)[(i)])
#define FD268(i) (((Func8008FDEntry *)&D_800FD818)[(i)])

typedef struct Func8008ResourceEntry {
    u8 *start;
    u8 *end;
    u8 unk8;
    u8 slot_index;
    u8 padA[4];
    s16 unkE;
    u32 flags;
} Func8008ResourceEntry;

#define RESOURCE14(i) (((Func8008ResourceEntry *)&g_field_resource_entries)[(i)])

/**
 * @brief Field actor per-frame state machine dispatch (opcodes 0x81..).
 *
 * Reads the actor's current opcode at offset 0x2A (biased by 0x81) and dispatches
 * to the matching movement, animation, path-following, resource-load, or transition
 * handler. Covers the large secondary opcode block for a field actor's active state.
 *
 * @param arg0 Pointer to the field actor state record (0x54-byte layout).
 * @note WIP m2c-derived match (96.25%); not yet byte-for-byte. Preserves original
 *       codegen forms (M2C_FIELD accesses, do/while(0) shells); do not clean up.
 */
void func_8008EF0C(void *arg0) {
    Func8008EF0CScratch scratch;
    s16 temp_a1;
    s16 temp_v1;
    s32 *var_a1_3;
    s32 temp_a0_3;
    s32 temp_a0_6;
    s32 temp_a0_7;
    s32 temp_a0_8;
    s32 temp_a1_2;
    s32 temp_a1_3;
    s32 temp_a2;
    s32 temp_a2_2;
    s32 temp_a3;
    s32 temp_a3_2;
    s32 temp_s0;
    s32 temp_s0_2;
    s32 temp_s0_3;
    s32 temp_s0_4;
    s32 temp_s0_5;
    s32 temp_s0_6;
    s32 temp_s0_7;
    s32 temp_s0_8;
    s32 temp_s0_9;
    s32 temp_v1_10;
    s32 temp_v1_9;
    s32 var_a0;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_a2;
    s32 var_a2_2;
    s32 var_s0;
    s32 var_s0_2;
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
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_7;
    s32 var_v0_8;
    s32 var_v0_9;
    s32 var_v1;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 var_v1_4;
    s8 temp_v0_30;
    s32 temp_v0_9;
    u16 *var_s3;
    s32 temp_v0_16;
    s32 temp_v0_18;
    s32 temp_v0_20;
    s32 temp_v0_8;
    u16 temp_v1_12;
    u32 temp_a1_5;
    u8 temp_a0_10;
    u8 temp_a0_12;
    u8 temp_a0_13;
    u8 temp_a0_14;
    u8 temp_a0_2;
    u8 temp_a0_5;
    u8 temp_a1_4;
    u8 temp_v0;
    u8 temp_v0_15;
    u8 temp_v0_22;
    u8 temp_v0_23;
    u8 temp_v0_24;
    u8 temp_v0_25;
    u8 temp_v0_27;
    u8 temp_v0_2;
    u8 temp_v0_3;
    u8 temp_v0_4;
    u8 temp_v0_5;
    u8 temp_v0_6;
    u8 temp_v1_13;
    u8 temp_v1_2;
    s32 temp_v1_4;
    u8 temp_v1_5;
    u8 temp_v1_6;
    u8 temp_v1_7;
    u8 var_v0_6;
    void *temp_a0;
    void *base37;
    void *base5fd;
    void *base5res;
    void *base05880;
    void *actors_base;
    void *slot_base;
    void *fd_base;
    s32 row4;
    void *temp_a0_11;
    void *temp_a0_4;
    void *temp_a0_9;
    void *temp_s2;
    void *temp_v0_10;
    void *temp_v0_11;
    void *temp_v0_12;
    void *temp_v0_13;
    void *temp_v0_14;
    void *temp_v0_17;
    void *temp_v0_19;
    void *temp_v0_21;
    void *temp_v0_26;
    void *temp_v0_28;
    void *temp_v0_29;
    void *temp_v0_31;
    void *temp_v0_32;
    void *temp_v0_33;
    void *temp_v0_34;
    void *temp_v0_35;
    void *temp_v0_36;
    void *temp_v0_37;
    void *temp_v0_38;
    void *temp_v0_7;
    void *temp_v1_11;
    void *temp_v1_3;
    void *temp_v1_8;

    var_s3 = (u16 *)0x801ED480;
    temp_s2 = (M2C_FIELD(arg0, u8 *, 0x3A) * 0x23C) + &D_80105AE0;
    temp_v1 = M2C_FIELD(arg0, u16 *, 0x2A) - 0x81;
    switch (temp_v1) {                              /* switch 1 */
    case 0x27:                                      /* switch 1 */
        if (M2C_FIELD(arg0, u16 *, 0x2E) == 0) {
            M2C_FIELD(arg0, u16 *, 0x2A) = 0U;
        }
        scratch.sp50 = 0;
        scratch.sp54 = 0;
        scratch.sp58 = 0;
        func_80097FA0(arg0, &scratch.sp50, 0);
        return;
    case 0x36:                                      /* switch 1 */
        if (M2C_FIELD(arg0, u16 *, 0x2E) == 0) {
            if ((M2C_FIELD(arg0, u8 *, 0x20) == 0) || (--M2C_FIELD(arg0, u8 *, 0x20) == 0)) {
                M2C_FIELD(arg0, u16 *, 0x2A) = 0U;
            }
        }
        return;
    case 0x32:                                      /* switch 1 */
block_229:
        M2C_FIELD(arg0, u16 *, 0x2A) = 0U;
        return;
    case 0x3A:                                      /* switch 1 */
        if ((M2C_FIELD(arg0, u8 *, 0x3D) != 0) && (--M2C_FIELD(arg0, u8 *, 0x3D) != 0)) {
            return;
        }
        if (field_object_has_active_actor_tracks(M2C_FIELD(arg0, u8 *, 0x3A)) != 0) {
            return;
        }
        goto block_set_ff;
    case 0x37:                                      /* switch 1 */
        if ((M2C_FIELD(arg0, u8 *, 0x3D) != 0) && (--M2C_FIELD(arg0, u8 *, 0x3D) != 0)) {
            goto block_37_record;
        }
        if (field_object_has_active_actor_tracks(M2C_FIELD(arg0, u8 *, 0x3A)) == 0) {
            goto block_37_cleanup;
        }
block_37_record:
        base37 = &D_800FDF58;
        temp_a0 = (M2C_FIELD(arg0, u8 *, 0x20) * 0x54) + base37;
        if (M2C_FIELD(temp_a0, u8 *, 0x25) == 0xFF) {
block_37_cleanup:
            field_stop_actor_animations_for_object(arg0, 0);
block_set_ff:
            M2C_FIELD(arg0, u8 *, 0x25) = 0xFF;
            return;
        }
        M2C_FIELD(arg0, s32 *, 0) = M2C_FIELD(temp_a0, s32 *, 0);
        M2C_FIELD(arg0, s32 *, 4) = M2C_FIELD(((M2C_FIELD(arg0, u8 *, 0x20) * 0x54) + base37), s32 *, 4);
        M2C_FIELD(arg0, s32 *, 8) = M2C_FIELD(((M2C_FIELD(arg0, u8 *, 0x20) * 0x54) + base37), s32 *, 8);
        return;
    case 0x5:                                       /* switch 1 */
        if (M2C_FIELD(arg0, u8 *, 0x3A) < 2U) {
            base5fd = &D_800FD818;
            temp_v1_3 = (M2C_FIELD(arg0, u8 *, 0x3A) * 0x268) + base5fd;
            if ((M2C_FIELD(temp_v1_3, u8 *, 3) == 0) && (M2C_FIELD(temp_v1_3, u8 *, 0x257) != 0)) {
                if ((func_80091728(M2C_FIELD(arg0, u8 *, 0x3A), 0U, arg0) != 0) && (func_80091728(M2C_FIELD(arg0, u8 *, 0x3A), 1U, arg0) != 0)) {
                    temp_a0_4 = &D_8010A038;
                    temp_v0_7 = (M2C_FIELD(arg0, u8 *, 0x3B) * 0x190) + temp_a0_4;
                    var_s0 = func_800AD7DC(M2C_FIELD(temp_v0_7, u16 *, 0), M2C_FIELD(temp_v0_7, u16 *, 8));
                    temp_a0_3 = var_s0 * 2;
                    if (var_s0 != 0xFF) {
                        scratch.sp48 = var_s0;
                        scratch.sp4C = (u16) *(temp_a0_3 + &D_800EB114);
                        scratch.sp4E = (s16) M2C_FIELD((&D_800EB114 + temp_a0_3), u8 *, 1);
                        switch (var_s0) {        /* switch 2 */
                        case 0x34:                  /* switch 2 */
                        case 0x3E:                  /* switch 2 */
                        case 0x45:                  /* switch 2 */
                        case 0x4E:                  /* switch 2 */
                        case 0x4F:                  /* switch 2 */
                        case 0x50:                  /* switch 2 */
                        case 0x51:                  /* switch 2 */
                            SLOT23C(M2C_FIELD(arg0, u8 *, 0x3A)).unk16F = (u8)var_s0;
                            break;
                        }
                        if (scratch.sp4C != 0xFFFF) {
                            if (scratch.sp4C != 0) {
                                var_s0 = func_800839F8(M2C_FIELD(arg0, u8 *, 0x3A), 0);
                                if (var_s0 != -1) {
                                    if (func_80083EEC(M2C_FIELD(arg0, u8 *, 0x3A), var_s0, scratch.sp4C) != 0) {
                                        field_start_actor_animation(var_s0, 0, 0);
                                        SLOT23C(M2C_FIELD(arg0, u8 *, 0x3A)).u178.b.b179 = (u8)var_s0;
                                    }
                                }
                            }
                        }
                        func_80090D48(arg0, temp_s2, &scratch.sp48);
                        FD268(M2C_FIELD(arg0, u8 *, 0x3A)).unk257 = 0;
                        var_s0 = 0;
                        func_800A2DD8(M2C_FIELD(arg0, u8 *, 0x3A));
                        func_8008BC5C(arg0);
                        temp_v1_4 = M2C_FIELD(arg0, u8 *, 0x3A);
                        if (SLOT23C(temp_v1_4).u178.b.b17B != 0) {
                            do {
                                temp_v1_4 = SLOT23C(temp_v1_4).unk180[var_s0];
                                SLOT23C(temp_v1_4).u178.word &= ~0x80;
                                temp_v1_4 = M2C_FIELD(arg0, u8 *, 0x3A);
                                var_s0 += 1;
                            } while (var_s0 < SLOT23C(temp_v1_4).u178.b.b17B);
                        }
                        SLOT23C(M2C_FIELD(arg0, u8 *, 0x3A)).u178.b.b17B = 0;
                        return;
                    }
                }
                temp_a0_11 = &D_800FD818;
                temp_v0_11 = (M2C_FIELD(arg0, u8 *, 0x3A) * 0x268) + temp_a0_11;
                M2C_FIELD(temp_v0_11, u8 *, 0x257) = (u8) (M2C_FIELD(temp_v0_11, u8 *, 0x257) - 1);
                goto block_42;
            }
        }
    case 0x15:                                      /* switch 1 */
        func_800925EC(arg0, 0);
        return;
    case 0x17:                                      /* switch 1 */
    case 0x1A:                                      /* switch 1 */
block_42:
        func_800925EC(arg0, 1);
        return;
    case 0x6:                                       /* switch 1 */
        func_80093EB4(arg0);
        return;
    case 0x10:                                      /* switch 1 */
        func_8009403C(arg0, M2C_FIELD(temp_s2, u16 *, 0x172));
        return;
    case 0x31:                                      /* switch 1 */
        if (!(RESOURCE14(M2C_FIELD(arg0, u8 *, 0x3B)).flags & 1)) {
            temp_v1_6 = *(M2C_FIELD(arg0, volatile u8 *, 0x20) + &D_800EB074);
            M2C_FIELD(arg0, u8 *, 0x20) = (u8) (M2C_FIELD(arg0, volatile u8 *, 0x20) + 1);
            M2C_FIELD(arg0, u8 *, 0x21) = temp_v1_6;
            if (*(M2C_FIELD(arg0, u8 *, 0x20) + &D_800EB074) == 0xFF) {
                M2C_FIELD(arg0, u16 *, 0x2A) = 0U;
                M2C_FIELD(arg0, u8 *, 0x20) = 0U;
            }
        } else {
            M2C_FIELD(arg0, u16 *, 0x2A) = 0U;
            M2C_FIELD(arg0, u8 *, 0x20) = 0U;
            M2C_FIELD(arg0, u8 *, 0x21) = (u8) (M2C_FIELD(arg0, u8 *, 0x21) ^ 0x80);
        }
        M2C_FIELD(arg0, s16 *, 0x2E) = 1;
        M2C_FIELD(arg0, s8 *, 0x27) = 0;
        M2C_FIELD(arg0, s8 *, 0x24) = 1;
        func_8006C3FC(arg0);
        return;
    case 0x19:                                      /* switch 1 */
        func_80092550(arg0);
        return;
    case 0x18:                                      /* switch 1 */
        func_800924D8(arg0);
        return;
    case 0x1:                                       /* switch 1 */
        func_800923F0(arg0);
        return;
    case 0xD: {
        u8 idxD;
        void *entryD;
        s16 limitD;
        void *baseD;
        baseD = &D_800FD818;
        idxD = M2C_FIELD(arg0, u8 *, 0x3A);
        entryD = (idxD * 0x268) + baseD;
        limitD = M2C_FIELD(entryD, s16 *, 0x260);
        if (limitD != 0) {
            if (idxD < 3U) {
                if (((M2C_FIELD(entryD, s16 *, 0x25E) >= limitD) || (M2C_FIELD(entryD, s16 *, 0x25E) = (s16) ((u16) M2C_FIELD(entryD, s16 *, 0x25E) + 1), temp_v0_12 = (M2C_FIELD(arg0, u8 *, 0x3A) * 0x268) + &D_800FD818, ((M2C_FIELD(temp_v0_12, s16 *, 0x25E) < M2C_FIELD(temp_v0_12, s16 *, 0x260)) == 0)))) {
                temp_v0_13 = (M2C_FIELD(arg0, u8 *, 0x3A) * 0x268) + &D_800FD818;
                M2C_FIELD(temp_v0_13, s16 *, 0x260) = 0;
                M2C_FIELD(temp_v0_13, s16 *, 0x25E) = 0;
                temp_a0_5 = M2C_FIELD(arg0, u8 *, 0x3A);
                temp_v0_14 = (temp_a0_5 * 0x268) + &D_800FD818;
                func_80089D44(temp_a0_5, M2C_FIELD(temp_v0_14, s16 *, 0x262), M2C_FIELD(temp_v0_14, s16 *, 0x264), M2C_FIELD(temp_v0_14, s16 *, 0x266));
                return;
                }
            }
        }
        break;
    }
    case 0xF:                                       /* switch 1 */
        func_80095168(arg0);
        return;
    case 0x11:                                      /* switch 1 */
        temp_a0 = &D_80105880;
        if ((u8) M2C_FIELD(arg0, u8 *, 0x3A) < 2U) {
            var_v0_2 = M2C_FIELD(arg0, u8 *, 0x3A) * 0x1C;
        } else {
            var_v0_2 = 0x38;
        }
        if (M2C_FIELD((temp_a0 + var_v0_2), s32 *, 0) != 0) {
            temp_a0 = &D_80105880;
            if ((u8) M2C_FIELD(arg0, u8 *, 0x3A) < 2U) {
                var_v0_3 = M2C_FIELD(arg0, u8 *, 0x3A) * 0x1C;
            } else {
                var_v0_3 = 0x38;
            }
            if (M2C_FIELD((temp_a0 + var_v0_3), s32 *, 0xC) == M2C_FIELD(arg0, u8 *, 0x3A)) {
                func_80094EA4(arg0);
                return;
            }
        }
        goto block_78;
    case 0x12:                                      /* switch 1 */
        temp_a0 = &D_80105880;
        if ((u8) M2C_FIELD(arg0, u8 *, 0x3A) < 2U) {
            var_v0_4 = M2C_FIELD(arg0, u8 *, 0x3A) * 0x1C;
        } else {
            var_v0_4 = 0x38;
        }
        if (M2C_FIELD((temp_a0 + var_v0_4), s32 *, 0) != 0) {
            temp_a0 = &D_80105880;
            if ((u8) M2C_FIELD(arg0, u8 *, 0x3A) < 2U) {
                var_v0_5 = M2C_FIELD(arg0, u8 *, 0x3A) * 0x1C;
            } else {
                var_v0_5 = 0x38;
            }
            if (M2C_FIELD((temp_a0 + var_v0_5), s32 *, 0xC) == M2C_FIELD(arg0, u8 *, 0x3A)) {
                func_80094F40(arg0);
                return;
            }
        }
block_78:
        func_8008404C(M2C_FIELD(arg0, u8 *, 0x3A), RESOURCE14(M2C_FIELD(arg0, u8 *, 0x3B)).unkE & 0x3FF);
        return;
    case 0x13:                                      /* switch 1 */
        func_80094FDC(arg0);
        return;
    case 0x2D:                                      /* switch 1 */
        var_v0_6 = M2C_FIELD(arg0, u8 *, 0x20);
        if (var_v0_6 != 0) {
            var_v0_6--;
            goto block_165;
        }
        if (!(M2C_FIELD(temp_s2, s32 *, 0x178) & 1)) {
            func_8008C104(arg0);
            return;
        }
        break;
    case 0x0:                                       /* switch 1 */
        var_a2 = 0;
block_138:
        func_80094508(arg0, 0, var_a2, 0);
        return;
    case 0xC:                                       /* switch 1 */
        temp_s0 = rcos(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4;
        func_800951CC(arg0, temp_s0, 0, (s32) -rsin(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4);
        return;
    case 0xE:                                       /* switch 1 */
        temp_s0_2 = rcos(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4;
        func_800949CC(arg0, temp_s0_2, 0, (s32) -rsin(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4);
        return;
    case 0x2E:                                      /* switch 1 */
        func_800946FC(arg0);
        return;
    case 0x2F:                                      /* switch 1 */
        var_v0_7 = M2C_FIELD((temp_s2 + (M2C_FIELD(temp_s2, u16 *, 0x1A6) << 3)), s32 *, 0x1AC) - M2C_FIELD(arg0, s32 *, 0);
        if (var_v0_7 < 0) {
            var_v0_7 += 0xFF;
        }
        scratch.sp50 = var_v0_7 >> 8;
        var_v0_8 = M2C_FIELD((temp_s2 + (M2C_FIELD(temp_s2, u16 *, 0x1A6) << 3)), s32 *, 0x1B0) - M2C_FIELD(arg0, s32 *, 8);
        if (var_v0_8 < 0) {
            var_v0_8 += 0xFF;
        }
        scratch.sp54 = var_v0_8 >> 8;
        gte_ldlvl((VECTOR *)&scratch.sp50);
        gte_sqr0();
        gte_stlvnl((VECTOR *)&scratch.sp60);
        if ((scratch.sp60 + scratch.sp64) < 0x32) {
            M2C_FIELD(arg0, s32 *, 0) = (s32) M2C_FIELD((temp_s2 + (M2C_FIELD(temp_s2, u16 *, 0x1A6) << 3)), s32 *, 0x1AC);
            M2C_FIELD(arg0, s32 *, 8) = (s32) M2C_FIELD((temp_s2 + (M2C_FIELD(temp_s2, u16 *, 0x1A6) << 3)), s32 *, 0x1B0);
            temp_v0_16 = M2C_FIELD(temp_s2, u16 *, 0x1A6) + 1;
            if (M2C_FIELD(temp_s2, u16 *, 0x1A4) != temp_v0_16) {
                M2C_FIELD(temp_s2, u16 *, 0x1A6) = temp_v0_16;
                return;
            }
            goto block_229;
        }
        temp_v0_17 = temp_s2 + (M2C_FIELD(temp_s2, u16 *, 0x1A6) << 3);
        var_v0_9 = ratan2(M2C_FIELD(arg0, s32 *, 8) - M2C_FIELD(temp_v0_17, s32 *, 0x1B0), M2C_FIELD(temp_v0_17, s32 *, 0x1AC) - M2C_FIELD(arg0, s32 *, 0)) >> 4;
        M2C_FIELD(arg0, u8 *, 0x1B) = (u8) var_v0_9;
        temp_s0_8 = rcos(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4;
        func_80094508(arg0, temp_s0_8, 0, (s32) -rsin(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4);
        return;
    case 0x7:                                       /* switch 1 */
        temp_s0_3 = rcos(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4;
        func_80094508(arg0, temp_s0_3, 0, (s32) -rsin(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4);
        return;
    case 0x30:                                      /* switch 1 */
        temp_v1_8 = temp_s2 + (M2C_FIELD(temp_s2, u16 *, 0x1A6) << 3);
        var_v0_10 = M2C_FIELD(temp_v1_8, s32 *, 0x1B0) - M2C_FIELD(arg0, s32 *, 8);
        temp_a0_6 = M2C_FIELD(temp_v1_8, s32 *, 0x1AC);
        if (var_v0_10 < 0) {
            var_v0_10 = -var_v0_10;
        }
        var_v1 = temp_a0_6 - M2C_FIELD(arg0, s32 *, 0);
        if (var_v1 < 0) {
            var_v1 = -var_v1;
        }
        if ((var_v0_10 + var_v1) < 0x2000) {
            M2C_FIELD(arg0, s32 *, 0) = temp_a0_6;
            M2C_FIELD(arg0, s32 *, 8) = (s32) M2C_FIELD((temp_s2 + (M2C_FIELD(temp_s2, u16 *, 0x1A6) << 3)), s32 *, 0x1B0);
            temp_v0_18 = M2C_FIELD(temp_s2, u16 *, 0x1A6) + 1;
            if (M2C_FIELD(temp_s2, u16 *, 0x1A4) != temp_v0_18) {
                M2C_FIELD(temp_s2, u16 *, 0x1A6) = temp_v0_18;
                return;
            }
            goto block_128;
        }
        M2C_FIELD(arg0, s8 *, 0x33) = 1;
        temp_v0_19 = temp_s2 + (M2C_FIELD(temp_s2, u16 *, 0x1A6) << 3);
        var_a1 = M2C_FIELD(arg0, s32 *, 0);
        var_v0_11 = M2C_FIELD(temp_v0_19, s32 *, 0x1AC);
        var_a0 = M2C_FIELD(arg0, s32 *, 8) - M2C_FIELD(temp_v0_19, s32 *, 0x1B0);
        var_a1 = var_v0_11 - var_a1;
        goto block_130;
    case 0x2C:                                      /* switch 1 */
        temp_s0_4 = rcos(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4;
        func_80094508(arg0, temp_s0_4, 0, (s32) -rsin(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4);
        if ((s16) M2C_FIELD(arg0, u16 *, 0x2A) == 0) {
            M2C_FIELD(arg0, s8 *, 0x33) = 0;
            return;
        }
        break;
    case 0x34:                                      /* switch 1 */
        var_v0_12 = M2C_FIELD((temp_s2 + (M2C_FIELD(temp_s2, u16 *, 0x1A6) << 3)), s32 *, 0x1AC) - M2C_FIELD(arg0, s32 *, 0);
        if (var_v0_12 < 0) {
            var_v0_12 += 0xFF;
        }
        scratch.sp50 = var_v0_12 >> 8;
        var_v0_13 = M2C_FIELD((temp_s2 + (M2C_FIELD(temp_s2, u16 *, 0x1A6) << 3)), s32 *, 0x1B0) - M2C_FIELD(arg0, s32 *, 8);
        if (var_v0_13 < 0) {
            var_v0_13 += 0xFF;
        }
        scratch.sp54 = var_v0_13 >> 8;
        gte_ldlvl((VECTOR *)&scratch.sp50);
        gte_sqr0();
        gte_stlvnl((VECTOR *)&scratch.sp60);
        if ((scratch.sp60 + scratch.sp64) < 0x32) {
            M2C_FIELD(arg0, s32 *, 0) = (s32) M2C_FIELD((temp_s2 + (M2C_FIELD(temp_s2, u16 *, 0x1A6) << 3)), s32 *, 0x1AC);
            M2C_FIELD(arg0, s32 *, 8) = (s32) M2C_FIELD((temp_s2 + (M2C_FIELD(temp_s2, u16 *, 0x1A6) << 3)), s32 *, 0x1B0);
            temp_v0_20 = M2C_FIELD(temp_s2, u16 *, 0x1A6) + 1;
            if (M2C_FIELD(temp_s2, u16 *, 0x1A4) == temp_v0_20) {
                M2C_FIELD(arg0, u16 *, 0x2A) = 0U;
            } else {
                M2C_FIELD(temp_s2, u16 *, 0x1A6) = temp_v0_20;
            }
        } else {
            temp_v0_21 = temp_s2 + (M2C_FIELD(temp_s2, u16 *, 0x1A6) << 3);
            M2C_FIELD(arg0, u8 *, 0x1B) = (u8) (ratan2(M2C_FIELD(arg0, s32 *, 8) - M2C_FIELD(temp_v0_21, s32 *, 0x1B0), M2C_FIELD(temp_v0_21, s32 *, 0x1AC) - M2C_FIELD(arg0, s32 *, 0)) >> 4);
            temp_s0_5 = rcos(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4;
            func_80094508(arg0, temp_s0_5, 0, (s32) -rsin(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4);
        }
        {
            s32 negB;
            s32 curB;
            s32 rawB;
            rawB = M2C_FIELD(var_s3, s32 *, 4);
            curB = M2C_FIELD(arg0, s32 *, 0);
            negB = -rawB;
            if ((negB + 0xA00) < curB) {
                if (curB < (negB + 0x13600)) {
                    rawB = M2C_FIELD(var_s3, s32 *, 0xC);
                    curB = M2C_FIELD(arg0, s32 *, 8);
                    negB = -rawB;
                    if ((negB + 0xA00) < curB) {
                        if (curB < (negB + 0x1B600)) {
                            M2C_FIELD(arg0, u16 *, 0x2A) = 0U;
                            return;
                        }
                    }
                }
            }
        }
        break;
    case 0xA:                                       /* switch 1 */
        temp_a3 = M2C_FIELD(temp_s2, s32 *, 0x58);
        temp_a2 = M2C_FIELD(arg0, s32 *, 8);
        do {
            temp_a0_7 = M2C_FIELD(temp_s2, s32 *, 0x50);
        } while (0);
        temp_v1_9 = M2C_FIELD(arg0, s32 *, 0);
        var_v0_14 = temp_a3 - temp_a2;
        if (var_v0_14 < 0) {
            var_v0_14 = -var_v0_14;
        }
        temp_a1_2 = temp_a0_7 - temp_v1_9;
        var_v1_2 = temp_a1_2;
        if (temp_a1_2 < 0) {
            var_v1_2 = -var_v1_2;
        }
        if ((var_v0_14 + var_v1_2) >= 0x1000) {
            var_v0_9 = ratan2(temp_a2 - temp_a3, temp_a1_2) >> 4;
                    M2C_FIELD(arg0, u8 *, 0x1B) = (u8) var_v0_9;
                    temp_s0_8 = rcos(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4;
                    func_80094508(arg0, temp_s0_8, 0, (s32) -rsin(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4);
                    return;
        }
        goto block_229;
    case 0x2B:                                      /* switch 1 */
        var_v0_15 = M2C_FIELD(temp_s2, s32 *, 0x58) - M2C_FIELD(arg0, s32 *, 8);
        if (var_v0_15 < 0) {
            var_v0_15 = -var_v0_15;
        }
        var_v1_3 = M2C_FIELD(temp_s2, s32 *, 0x50) - M2C_FIELD(arg0, s32 *, 0);
        if (var_v1_3 < 0) {
            var_v1_3 = -var_v1_3;
        }
        if ((var_v0_15 + var_v1_3) < 0x1000) {
block_128:
            M2C_FIELD(arg0, u16 *, 0x2A) = 0U;
            M2C_FIELD(arg0, s8 *, 0x33) = 0;
            return;
        }
        M2C_FIELD(arg0, s8 *, 0x33) = 1;
        var_a0 = M2C_FIELD(arg0, s32 *, 8) - M2C_FIELD(temp_s2, s32 *, 0x58);
        var_v0_11 = M2C_FIELD(temp_s2, s32 *, 0x50);
        var_a1 = M2C_FIELD(arg0, s32 *, 0);
        var_a1 = var_v0_11 - var_a1;
block_130:
        M2C_FIELD(arg0, u8 *, 0x1B) = (u8) (ratan2(var_a0, var_a1) >> 4);
        temp_s0_8 = rcos(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4;
        func_80094508(arg0, temp_s0_8, 0, (s32) -rsin(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4);
        if ((s16) M2C_FIELD(arg0, u16 *, 0x2A) == 0) {
            M2C_FIELD(arg0, s8 *, 0x33) = 0;
            return;
        }
        break;
    case 0xB: {                                     /* switch 1 */
        s32 target_z;
        s32 current_z;
        s32 target_x;
        s32 current_x;
        target_z = M2C_FIELD(temp_s2, s32 *, 0x58);
        current_z = M2C_FIELD(arg0, s32 *, 8);
        target_x = M2C_FIELD(temp_s2, s32 *, 0x50);
        current_x = M2C_FIELD(arg0, s32 *, 0);
        var_v0_9 = (ratan2(current_z - target_z, target_x - current_x) >> 4) - 0x80;
block_134:
        M2C_FIELD(arg0, u8 *, 0x1B) = (u8) var_v0_9;
        temp_s0_8 = rcos(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4;
        func_80094508(arg0, temp_s0_8, 0, (s32) -rsin(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4);
        return;
    }
    case 0x8:                                       /* switch 1 */
        func_80094508(arg0, 0, -0x100, 0);
        return;
    case 0x9:                                       /* switch 1 */
        func_80094508(arg0, 0, 0x100, 0);
        return;
    case 0x1B:                                      /* switch 1 */
        func_80094B5C(arg0, 1);
        return;
    case 0x1C:                                      /* switch 1 */
        func_80094B5C(arg0, 0);
        return;
    case 0x1F:                                      /* switch 1 */
        temp_a3 = M2C_FIELD(temp_s2, s32 *, 0x58);
        temp_a2 = M2C_FIELD(arg0, s32 *, 8);
        do {
            temp_a0_7 = M2C_FIELD(temp_s2, s32 *, 0x50);
        } while (0);
        temp_v1_9 = M2C_FIELD(arg0, s32 *, 0);
        var_v0_16 = temp_a3 - temp_a2;
        if (var_v0_16 < 0) {
            var_v0_16 = -var_v0_16;
        }
        temp_a1_3 = temp_a0_7 - temp_v1_9;
        var_v1_4 = temp_a1_3;
        if (temp_a1_3 < 0) {
            var_v1_4 = -var_v1_4;
        }
        if ((var_v0_16 + var_v1_4) >= 0x2000) {
            M2C_FIELD(arg0, u8 *, 0x1B) = (u8) (ratan2(temp_a2 - temp_a3, temp_a1_3) >> 4);
            temp_s0_6 = rcos(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4;
            func_80094BC4(arg0, temp_s0_6, (s32) -rsin(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4);
            return;
        }
        goto block_229;
    case 0x26:                                      /* switch 1 */
        temp_s0_7 = rcos(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4;
        func_80094C00(arg0, temp_s0_7, (s32) -rsin(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4);
        if ((M2C_FIELD(temp_s2, u8 *, 0x171) == 0) || (--M2C_FIELD(temp_s2, u8 *, 0x171) == 0)) {
            M2C_FIELD(arg0, u16 *, 0x2A) = 0U;
        }
        return;
    case 0x35:                                      /* switch 1 */
        temp_s0_8 = rcos(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4;
        func_80094690(arg0, temp_s0_8, (s32) -rsin(M2C_FIELD(arg0, u8 *, 0x1B) * 0x10) >> 4);
        if ((M2C_FIELD(temp_s2, u8 *, 0x171) == 0) || (--M2C_FIELD(temp_s2, u8 *, 0x171) == 0)) {
            M2C_FIELD(arg0, u16 *, 0x2A) = 0U;
        }
        return;
    case 0x3B:                                      /* switch 1 */
        actors_base = &g_field_actor_slots;
        base05880 = &D_80105880;
        if ((u8) M2C_FIELD(arg0, u8 *, 0x3A) < 2U) {
            var_v0_17 = M2C_FIELD(arg0, u8 *, 0x3A) * 0x1C;
        } else {
            var_v0_17 = 0x38;
        }
        if (M2C_FIELD((actors_base + (M2C_FIELD((base05880 + var_v0_17), s32 *, 0x18) * 0x244)), u8 *, 0x24) == 0) {
            M2C_FIELD(arg0, u16 *, 0x2A) = 0U;
            return;
        }
        break;
    case 0x3C:                                      /* switch 1 */
        if (M2C_FIELD(arg0, u16 *, 0x2E) == 0) {
            temp_a1_4 = M2C_FIELD(arg0, u8 *, 0x21);
            if ((temp_a1_4 & 0x7F) == 0xF) {
                slot_base = &D_80105AE0;
                M2C_FIELD(arg0, u8 *, 0x21) = (u8) ((temp_a1_4 & 0x80) + 0x31);
                M2C_FIELD(arg0, u16 *, 0x2E) = 1U;
                M2C_FIELD(arg0, s8 *, 0x27) = 0;
                M2C_FIELD(arg0, s8 *, 0x24) = 1;
                temp_v0_26 = (M2C_FIELD(arg0, u8 *, 0x3A) * 0x23C) + slot_base;
                M2C_FIELD(temp_v0_26, s32 *, 0x174) = (s32) (M2C_FIELD(temp_v0_26, s32 *, 0x174) & ~0x1800);
                func_8006C3FC(arg0);
                return;
            }
            goto block_229;
        }
        break;
    case 0x23:                                      /* switch 1 */
        if (M2C_FIELD(arg0, u16 *, 0x2E) == 0) {
            M2C_FIELD(arg0, u16 *, 0x2A) = 0U;
            M2C_FIELD(arg0, u16 *, 0x2E) = 1U;
            M2C_FIELD(arg0, s8 *, 0x24) = 1;
            M2C_FIELD(arg0, u8 *, 0x21) = (u8) (M2C_FIELD(arg0, u8 *, 0x21) & 0x80);
            func_8006C3FC(arg0);
            return;
        }
        break;
    case 0x14:                                      /* switch 1 */
        var_v0_6 = M2C_FIELD(arg0, u8 *, 0x20);
        if (var_v0_6 != 0) {
            var_v0_6--;
            goto block_165;
        }
        func_80096334(arg0);
        M2C_FIELD(arg0, u16 *, 0x2A) = 0U;
        func_800A2DD8(M2C_FIELD(arg0, u8 *, 0x3A));
        return;
block_165:
        M2C_FIELD(arg0, u8 *, 0x20) = var_v0_6;
        return;
    case 0x4: {                                     /* switch 1 */
        void *base4;
        base4 = &D_80105AE0;
        M2C_FIELD(((M2C_FIELD(arg0, u8 *, 0x3A) * 0x23C) + base4), s8 *, 0x17B) = 0;
        if ((u8) M2C_FIELD(arg0, u8 *, 0x3A) < 3U) {
            fd_base = &D_800FD818;
            M2C_FIELD(((M2C_FIELD(arg0, u8 *, 0x3A) * 0x268) + fd_base), s8 *, 0x257) = 0;
        }
        if (M2C_FIELD(arg0, s32 *, 0x1C) & 0x1FF) {
            row4 = M2C_FIELD(arg0, u8 *, 0x3B) * 0x190;
            var_s3 = row4 + ((M2C_FIELD(((M2C_FIELD(arg0, u8 *, 0x3A) * 0x23C) + base4), u8 *, 0x16F) * 8) + &D_8010A038);
        } else {
            temp_a0_9 = (M2C_FIELD(arg0, u8 *, 0x3A) * 0x23C) + base4;
            if ((u8) M2C_FIELD(temp_a0_9, u8 *, 0x16F) < 0xCU) {
                do {
                row4 = M2C_FIELD(arg0, u8 *, 0x3B) * 0x190;
            } while (0);
                var_s3 = row4 + ((M2C_FIELD(temp_a0_9, u8 *, 0x16F) * 8) + &D_8010A038);
            } else {
                var_s3 = (M2C_FIELD(arg0, u8 *, 0x3B) * 0x190) + &D_8010A090;
            }
        }
    }
        if (M2C_FIELD(var_s3, u8 *, 2) != 0xFF) {
            s32 result4;
            temp_s0_9 = func_8009D1E4(M2C_FIELD(arg0, u8 *, 0x3A), var_s3, (M2C_FIELD(var_s3, u16 *, 2) >> 8) & 3, M2C_FIELD(temp_s2, u16 *, 0x174) & 0x3FF, &scratch.sp18);
            result4 = 0;
            if (D_8010AE54 == 0) {
                if (!(M2C_FIELD(arg0, s32 *, 0x1C) & 0x1FF)) {
                    temp_a0_10 = M2C_FIELD(arg0, u8 *, 0x3A);
                    temp_v1_11 = (temp_a0_10 * 0x23C) + &D_80105AE0;
                    if ((u8) M2C_FIELD(temp_v1_11, u8 *, 0x16F) < 0xCU) {
                        result4 = func_80091728(temp_a0_10, M2C_FIELD(temp_v1_11, u8 *, 0x16F), arg0);
                    }
                } else {
                    result4 = func_80090F50(arg0, 0);
                }
            }
            if (result4 != 0) {
                temp_s0_9 = (s32)&D_80105AE0;
                temp_v0_28 = (M2C_FIELD(arg0, u8 *, 0x3A) * 0x23C) + (void *)temp_s0_9;
                M2C_FIELD(temp_v0_28, s32 *, 0x178) = (s32) (M2C_FIELD(temp_v0_28, s32 *, 0x178) | 0x40);
                func_8009D9E0(arg0, M2C_FIELD(var_s3, u8 *, 2));
                if ((M2C_FIELD(var_s3, u16 *, 2) & 0x400) && (M2C_FIELD(arg0, u16 *, 0x2E) == 0)) {
                    M2C_FIELD(arg0, u16 *, 0x2E) = 1U;
                    M2C_FIELD(arg0, s8 *, 0x24) = 1;
                    M2C_FIELD(arg0, s8 *, 0x27) = 0;
                    M2C_FIELD(arg0, u8 *, 0x21) = (u8) ((M2C_FIELD(arg0, u8 *, 0x21) & 0x80) + 0xE);
                    temp_v0_29 = (M2C_FIELD(arg0, u8 *, 0x3A) * 0x23C) + (void *)temp_s0_9;
                    M2C_FIELD(temp_v0_29, s32 *, 0x174) = (s32) (M2C_FIELD(temp_v0_29, s32 *, 0x174) & ~0x1800);
                    func_8006C3FC(arg0, -0x1801);
                }
                temp_s2 = &D_80105AE0;
                temp_a0_11 = (M2C_FIELD(arg0, u8 *, 0x3A) * 0x23C) + temp_s2;
                temp_a1_5 = M2C_FIELD(temp_a0_11, u32 *, 0x174);
                if (!((temp_a1_5 >> 0xA) & 1) && ((u16) M2C_FIELD(temp_a0_11, u16 *, 0x4A) >= 0x41U)) {
                    temp_v1_12 = M2C_FIELD(var_s3, u16 *, 4);
                    if ((temp_v1_12 != 0xFFFF) && (temp_v1_12 != 0)) {
                        M2C_FIELD(temp_a0_11, u32 *, 0x174) = (u32) (temp_a1_5 | 0x400);
                        temp_s0_9 = func_800839F8(M2C_FIELD(arg0, u8 *, 0x3A), 0);
                        if ((temp_s0_9 != -1) && (func_80083EEC(M2C_FIELD(arg0, u8 *, 0x3A), temp_s0_9, M2C_FIELD(var_s3, u16 *, 4)) != 0)) {
                            field_start_actor_animation(temp_s0_9, 0, 0);
                            M2C_FIELD(((M2C_FIELD(arg0, u8 *, 0x3A) * 0x23C) + temp_s2), s8 *, 0x179) = temp_s0_9;
                            temp_a0_12 = M2C_FIELD(arg0, u8 *, 0x3A);
                            func_800A623C(temp_a0_12, (temp_a0_12 << 0x10) | ((M2C_FIELD(((temp_a0_12 * 0x23C) + temp_s2), u8 *, 0x16F) - 4) | 0x80000000));
                            return;
                        }
                    }
                }
            } else {
                temp_v0_31 = (M2C_FIELD(arg0, u8 *, 0x3A) * 0x23C) + &D_80105AE0;
                M2C_FIELD(temp_v0_31, s32 *, 0x178) = (s32) (M2C_FIELD(temp_v0_31, s32 *, 0x178) & ~0x40);
                temp_v1_13 = M2C_FIELD(arg0, u8 *, 0x3A);
                if (!(((u32) M2C_FIELD(((temp_v1_13 * 0x23C) + &D_80105AE0), u32 *, 0x174) >> 0xA) & 1)) {
                    actors_base = &g_field_actor_slots;
                    base05880 = &D_80105880;
                    if (temp_v1_13 < 2U) {
                        var_v0_18 = temp_v1_13 * 0x1C;
                    } else {
                        var_v0_18 = 0x38;
                    }
                    M2C_FIELD((actors_base + (M2C_FIELD((base05880 + var_v0_18), s32 *, 0x18) * 0x244)), s8 *, 0x24) = 0;
                    func_80084424(M2C_FIELD(arg0, u8 *, 0x3A));
                    slot_base = &D_80105AE0;
                    M2C_FIELD(arg0, u16 *, 0x2E) = 1U;
                    M2C_FIELD(arg0, s8 *, 0x24) = 1;
                    M2C_FIELD(arg0, s8 *, 0x27) = 0;
                    M2C_FIELD(arg0, u8 *, 0x21) = (u8) ((M2C_FIELD(arg0, u8 *, 0x21) & 0x80) + 0xF);
                    temp_v0_32 = (M2C_FIELD(arg0, u8 *, 0x3A) * 0x23C) + slot_base;
                    M2C_FIELD(temp_v0_32, s32 *, 0x174) = (s32) (M2C_FIELD(temp_v0_32, s32 *, 0x174) & ~0x1800);
                    func_8006C3FC(arg0);
                    M2C_FIELD(arg0, u16 *, 0x2A) = 0xBDU;
                    return;
                }
                base05880 = &D_80105880;
                if (temp_v1_13 < 2U) {
                    var_v0_19 = temp_v1_13 * 0x1C;
                } else {
                    var_v0_19 = 0x38;
                }
                var_s0_2 = func_80090B38(temp_s0_9, &scratch.sp18, M2C_FIELD((base05880 + var_v0_19), s32 *, 0x18));
                actors_base = &g_field_actor_slots;
                base05880 = &D_80105880;
                if ((u8) M2C_FIELD(arg0, u8 *, 0x3A) < 2U) {
                    var_v0_20 = M2C_FIELD(arg0, u8 *, 0x3A) * 0x1C;
                } else {
                    var_v0_20 = 0x38;
                }
                temp_v0_33 = actors_base + (M2C_FIELD((base05880 + var_v0_20), s32 *, 0x18) * 0x244);
                M2C_FIELD(temp_v0_33, s32 *, 0x224) = (s32) ((M2C_FIELD(temp_v0_33, s32 *, 0x224) & ~0x1E) | ((M2C_FIELD(var_s3, u8 *, 2) & 0xF) * 2));
                if (!(M2C_FIELD(var_s3, u16 *, 6) & 0x800)) {
                    if (func_80090F80(M2C_FIELD(arg0, u8 *, 0x3A), 0, NULL, M2C_FIELD(var_s3, u16 *, 6)) == 0) {
                        goto block_212;
                    }
                    goto block_217;
                }
                if (var_s0_2 != 0) {
                    var_a1_2 = var_s0_2;
                    if (var_s0_2 >= 0xA) {
                        var_s0_2 = 9;
                        var_a1_2 = 9;
                    }
                    if (func_80090F80(M2C_FIELD(arg0, u8 *, 0x3A), var_a1_2, &scratch.sp18, M2C_FIELD(var_s3, u16 *, 6)) != 0) {
                        var_a2_2 = 0;
                        if (var_s0_2 > 0) {
                            var_a1_3 = &scratch.sp18;
                            do {
                                temp_v0_34 = (*var_a1_3 * 0x23C) + &D_80105AE0;
                                M2C_FIELD(temp_v0_34, s32 *, 0x178) = (s32) (M2C_FIELD(temp_v0_34, s32 *, 0x178) | 0x80);
                                temp_v0_35 = (M2C_FIELD(arg0, u8 *, 0x3A) * 0x23C) + &D_80105AE0;
                                M2C_FIELD((temp_v0_35 + M2C_FIELD(temp_v0_35, u8 *, 0x17B)), u8 *, 0x180) = (u8) *var_a1_3;
                                var_a2_2 += 1;
                                temp_v0_36 = (M2C_FIELD(arg0, u8 *, 0x3A) * 0x23C) + &D_80105AE0;
                                M2C_FIELD(temp_v0_36, u8 *, 0x17B) = (u8) (M2C_FIELD(temp_v0_36, u8 *, 0x17B) + 1);
                                var_a1_3 += 1;
                            } while (var_a2_2 < var_s0_2);
                        }
                        goto block_217;
                    }
block_212:
                    temp_v0_37 = (M2C_FIELD(arg0, u8 *, 0x3A) * 0x23C) + &D_80105AE0;
                    M2C_FIELD(temp_v0_37, s32 *, 0x178) = (s32) (M2C_FIELD(temp_v0_37, s32 *, 0x178) | 0x40);
                    return;
                }
                actors_base = &g_field_actor_slots;
                base05880 = &D_80105880;
                if ((u8) M2C_FIELD(arg0, u8 *, 0x3A) < 2U) {
                    var_v0_21 = M2C_FIELD(arg0, u8 *, 0x3A) * 0x1C;
                } else {
                    var_v0_21 = 0x38;
                }
                M2C_FIELD((actors_base + (M2C_FIELD((base05880 + var_v0_21), s32 *, 0x18) * 0x244)), s8 *, 0x24) = 0;
                func_80084424(M2C_FIELD(arg0, u8 *, 0x3A));
                M2C_FIELD(((M2C_FIELD(arg0, u8 *, 0x3A) * 0x23C) + &D_80105AE0), s8 *, 0x179) = 0xFF;
                goto block_217;
            }
        } else {
block_217:
            if (M2C_FIELD(var_s3, u16 *, 2) & 0x400) {
                slot_base = &D_80105AE0;
                M2C_FIELD(arg0, u16 *, 0x2A) = 0x87U;
                M2C_FIELD(arg0, u16 *, 0x2E) = 1U;
                M2C_FIELD(arg0, s8 *, 0x24) = 1;
                M2C_FIELD(arg0, s8 *, 0x27) = 0;
                M2C_FIELD(arg0, u8 *, 0x21) = (u8) ((M2C_FIELD(arg0, u8 *, 0x21) & 0x80) + 0xF);
                temp_v0_38 = (M2C_FIELD(arg0, u8 *, 0x3A) * 0x23C) + slot_base;
                M2C_FIELD(temp_v0_38, s32 *, 0x174) = (s32) (M2C_FIELD(temp_v0_38, s32 *, 0x174) & ~0x1800);
                goto block_223;
            }
            if (M2C_FIELD(var_s3, u16 *, 0) & 0x8000) {
                M2C_FIELD(((M2C_FIELD(arg0, u8 *, 0x3A) * 0x23C) + &D_80105AE0), s16 *, 0x48) = 0;
                M2C_FIELD(temp_s2, s32 *, 0x44) = 0;
                M2C_FIELD(temp_s2, s32 *, 0x40) = (s32) M2C_FIELD(var_s3, u16 *, 6);
                M2C_FIELD(temp_s2, u16 *, 0x174) = (s32) ((s32) M2C_FIELD(temp_s2, u16 *, 0x174) & ~0x1800);
                M2C_FIELD(temp_s2, s32 *, 0x3C) = (s32) M2C_FIELD(var_s3, u16 *, 4);
                M2C_FIELD(temp_s2, s32 *, 0x40) = (s32) ((((M2C_FIELD(var_s3, u16 *, 0) & 0x7FFF) + 0x88) | 0x8000) + (M2C_FIELD(((M2C_FIELD(arg0, u8 *, 0x3A) * 0x268) + &D_800FD818), u8 *, 1) * 0x18));
                if ((u8) M2C_FIELD(arg0, u8 *, 0x3A) < 3U) {
                    temp_a0_13 = M2C_FIELD(arg0, u8 *, 0x3A);
                    func_800A623C(temp_a0_13, (M2C_FIELD(var_s3, u16 *, 0) & 0x7FFF) + (M2C_FIELD(((temp_a0_13 * 0x268) + &D_800FD818), u8 *, 1) * 0x18));
                }
                func_800954F0(arg0, M2C_FIELD(var_s3, u16 *, 0) & 0x7FFF);
                M2C_FIELD(temp_s2, u16 *, 0x172) = (u16) (M2C_FIELD(var_s3, u16 *, 0) & 0x7FFF);
                M2C_FIELD(arg0, u16 *, 0x2A) = 0x91U;
block_223:
                func_8006C3FC(arg0);
                M2C_FIELD(arg0, s32 *, 0x1C) = (s32) (M2C_FIELD(arg0, s32 *, 0x1C) | 0x800);
                return;
            }
            if ((u8) M2C_FIELD(arg0, u8 *, 0x3A) < 3U) {
                temp_a0_14 = M2C_FIELD(arg0, u8 *, 0x3A);
                if ((u8) M2C_FIELD(((temp_a0_14 * 0x23C) + &D_80105AE0), u8 *, 0x16F) < 2U) {
                    M2C_FIELD(((temp_a0_14 * 0x268) + &D_800FD818), s8 *, 0x257) = 0xA;
                }
            }
            func_80090D48(arg0, temp_s2, var_s3);
            return;
        }
        break;
    default:                                        /* switch 1 */
        scratch.sp50 = 0;
        scratch.sp54 = 0;
        scratch.sp58 = 0;
        func_80097FA0(arg0, &scratch.sp50, 0);
        M2C_FIELD(arg0, u16 *, 0x2A) = 0U;
        return;
    }
}
