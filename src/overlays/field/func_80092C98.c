#include "common.h"

/** @brief Field actor record (0x54 bytes); only the fields this handler touches are named. */

typedef struct
{
    s32 unk0;
    s32 unk4;
    u8 pad8[0x16 - 0x8];
    s16 unk16;
    u8 pad18[0x1C - 0x18];
    s32 unk1C;
    u8 unk20;
    u8 unk21;
    u8 pad22[0x27 - 0x22];
    u8 unk27;
    u8 pad28[0x2A - 0x28];
    s16 unk2A;
    u8 pad2C[0x34 - 0x2C];
    u8 unk34;
    u8 unk35;
    u8 unk36;
    s8 unk37;
    s8 unk38;
    u8 unk39;
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} FieldRec;

/** @brief D_80105AE0 slot record (stride 0x23C). */
typedef struct
{
    u8 pad0[0xC];
    s32 unkC;
    u8 pad10[0x170 - 0x10];
    u8 unk170;
    u8 pad171[0x178 - 0x171];
    u32 unk178;
    u8 pad17C[0x23C - 0x17C];
} Slot23C;

/** @brief D_800FD818 object entry (stride 0x268). */
typedef struct
{
    u8 unk0;
    u8 unk1;
    u8 pad2[0x268 - 0x2];
} Entry268;

/** @brief D_8010A038 animation record (stride 0x190); unk5A is written as both a u16 and its low byte. */
typedef struct
{
    u16 unk0;
    u8 pad2[0x8 - 0x2];
    u16 unk8;
    u8 padA[0x58 - 0xA];
    u16 unk58;
    union
    {
        u16 h;
        struct
        {
            u8 lo;
            u8 hi;
        } b;
    } unk5A;
    u16 unk5C;
    u16 unk5E;
    u8 pad60[0x190 - 0x60];
} Anim190;

extern Slot23C D_80105AE0[];
extern Entry268 D_800FD818[];
extern Anim190 D_8010A038[];

void field_start_actor_animation(s32 slot_index, s32 target_count, s32 *targets);
void func_8006C3FC(FieldRec *rec);
s32 func_800839F8(s32 arg0, s32 arg1);
s32 func_80083EEC(u8 object_index, s32 actor_index, s32 animation_id);
void func_8008A9D8(s32 arg0, s32 arg1, s32 arg2);
s32 func_8008AABC(s32 a, s32 b);
void func_8008BC5C(FieldRec *rec);
void func_8008E690(FieldRec *rec);
s32 func_80091728(u8 index, s32 kind, FieldRec *rec);
s32 func_80091914(FieldRec *rec, u8 index);
void func_80096334(FieldRec *rec);
void func_800A2DD8(u8 index);

/** @brief Program the animation record for object @p idx (fields 0x58..0x5E). */
#define SET_ANIM(idx, v58, v5C, v5E)                                  \
    D_8010A038[idx].unk5A.h &= 0xFBFF;                                \
    D_8010A038[idx].unk58 = v58;                                      \
    D_8010A038[idx].unk5A.b.lo = 0xFF;                                \
    D_8010A038[idx].unk5C = v5C;                                      \
    D_8010A038[idx].unk5E = v5E;                                      \
    D_8010A038[idx].unk5A.h &= 0xFCFF;

/** @brief Interpolated step offset (unk37..unk38 scaled by unk34/unk35), in 1/256 units. */
#define STEP_OFFSET(rec) \
    ((rec->unk37 + (rec->unk38 - rec->unk37) * rec->unk34 / rec->unk35) << 8)

/**
 * @brief Per-frame state handler for a field actor's opcode 0x86 / trigger-kind states.
 *
 * With no pending flags in unk1C, first resolves the 0x3D transition when the
 * current animation matches, then dispatches on the opcode (unk21 & 0x7F) by
 * trigger kind (func_80091728 kinds 3, 1/0, 2), programming the D_8010A038
 * animation record and queueing the follow-up state via func_8008E690.
 *
 * @param rec Field actor record.
 * @return Never set; the declared non-void return keeps v0 live at the epilogue,
 *         which is what the original codegen shows (all exits are bare returns).
 * @note 97.53% (830/904 rows). Remaining residue: (1) the second 0x3D compare
 *       re-reads D_8010A038[unk3A].unk0 and compares against 0x3D in the target,
 *       ours CSEs it against the first read (9 target-only rows); (2) the
 *       D_800FD818 index load is shared with the func_80083EEC first argument in
 *       the target (register a0), ours reloads it (2 sites); (3) the
 *       tmp * 8 + unk3A * 0x190 offset sum has swapped operands at 2 sites.
 * @note The jump table jtbl_80050F14 lives in the FIELD data region
 *       (asm/overlays/field/unk2.s), not in this function's .rodata; placing it
 *       correctly in the final link still needs handling.
 * @see decomp.me (97.53%) TODO
 */
s32 func_80092C98(FieldRec *rec)
{
    s32 targets;
    s32 tmp;
    s32 anim;
    s32 anim_id;

    if (rec->unk1C & 0x1FF)
    {
        return;
    }
    if (rec->unk2A == 0x86)
    {
        tmp = rec->unk21 & 0x7F;
        if (tmp == 0x3D)
        {
            anim = func_80091914(rec, rec->unk3A);
            if ((D_8010A038[rec->unk3A].unk8 == tmp && anim == 0x185) ||
                (D_8010A038[rec->unk3A].unk0 == 0x3D && anim == 0x85))
            {
                rec->unk2A = anim;
                rec->unk4 -= STEP_OFFSET(rec);
                func_8008E690(rec);
                func_800A2DD8(rec->unk3A);
                rec->unk2A = 0x9B;
                return;
            }
        }
    }
    if (func_80091728(rec->unk3A, 3, rec) != 0)
    {
        switch (rec->unk21 & 0x7F)
        {
        case 0x2F:
        case 0x44:
            rec->unk2A = 0x885;
            func_8008E690(rec);
            func_800A2DD8(rec->unk3A);
            break;
        case 0x3E:
            rec->unk2A = 0xA85;
            func_8008E690(rec);
            func_800A2DD8(rec->unk3A);
            break;
        case 0x38:
            rec->unk2A = 0xA85;
            func_8008E690(rec);
            func_800A2DD8(rec->unk3A);
            break;
        case 0x3A:
            SET_ANIM(rec->unk3A, 0x4F, 0x25, 0);
            rec->unk2A = 0xB85;
            func_8008E690(rec);
            func_800A2DD8(rec->unk3A);
            break;
        case 0x39:
            SET_ANIM(rec->unk3A, 0x4F, 0x25, 0);
            rec->unk2A = 0xB85;
            func_8008E690(rec);
            func_800A2DD8(rec->unk3A);
            break;
        case 0x34:
            SET_ANIM(rec->unk3A, 0x51, 0x27, 0);
            rec->unk2A = 0xB85;
            func_8008E690(rec);
            func_800A2DD8(rec->unk3A);
            break;
        case 0x8:
        case 0x3B:
        case 0x3C:
        case 0x3D:
            if (rec->unk27 < 3)
            {
                return;
            }
            rec->unk21 = (rec->unk21 & 0x80) | 0x49;
            rec->unk4 -= STEP_OFFSET(rec);
            func_8006C3FC(rec);
            func_800A2DD8(rec->unk3A);
            rec->unk2A = 0x96;
            rec->unk16 = 1;
            rec->unk34 = 1;
            rec->unk35 = 1;
            return;
        case 0x35:
            if ((D_80105AE0[rec->unk3A].unk178 >> 1) & 1)
            {
                D_80105AE0[D_80105AE0[rec->unk3A].unk170].unkC &= ~0x2000;
                rec->unk2A = 0;
                func_80096334(rec);
                tmp = func_800839F8(rec->unk3A, 0);
                if (tmp != -1)
                {
                    if (func_8008AABC(rec->unk3A, D_80105AE0[rec->unk3A].unk170) != 0)
                    {
                        if (D_800FD818[rec->unk3A].unk1 == 8)
                        {
                            func_8008A9D8(rec->unk3A, D_80105AE0[rec->unk3A].unk170, 0xD);
                        }
                        else
                        {
                            func_8008A9D8(rec->unk3A, D_80105AE0[rec->unk3A].unk170, 0xC);
                        }
                        anim_id = 0x64;
                        if (D_800FD818[rec->unk3A].unk1 == 8)
                        {
                            anim_id = 0x61;
                        }
                    }
                    else
                    {
                        if (D_800FD818[rec->unk3A].unk1 == 8)
                        {
                            func_8008A9D8(D_80105AE0[rec->unk3A].unk170, rec->unk3A, 0x18);
                        }
                        else
                        {
                            func_8008A9D8(D_80105AE0[rec->unk3A].unk170, rec->unk3A, 0x17);
                        }
                        anim_id = 0x65;
                        if (D_800FD818[rec->unk3A].unk1 == 8)
                        {
                            anim_id = 0x63;
                        }
                    }
                    if (func_80083EEC(rec->unk3A, tmp, anim_id) != 0)
                    {
                        targets = D_80105AE0[rec->unk3A].unk170;
                        field_start_actor_animation(tmp, 1, &targets);
                    }
                    func_800A2DD8(rec->unk3A);
                }
                func_8008BC5C(rec);
            }
            return;
        default:
            return;
        }
    }
    else if (func_80091728(rec->unk3A, 1, rec) != 0 || func_80091728(rec->unk3A, 0, rec) != 0)
    {
        tmp = func_80091728(rec->unk3A, 1, rec) != 0;
        switch (rec->unk21 & 0x7F)
        {
        case 0x25:
            anim = *(u16 *)(tmp * 8 + rec->unk3A * 0x190 + (u8 *)D_8010A038);
            if (anim == 8 || anim == 0x3C)
            {
                rec->unk2A = 0x985;
                func_8008E690(rec);
                func_800A2DD8(rec->unk3A);
            }
            break;
        case 0x31:
            if (*(u16 *)(tmp * 8 + rec->unk3A * 0x190 + (u8 *)D_8010A038) == 8)
            {
                SET_ANIM(rec->unk3A, 0x3C, 0, 1);
                rec->unk2A = 0xB85;
                func_8008E690(rec);
                func_800A2DD8(rec->unk3A);
            }
            break;
        }
    }
    else if (func_80091728(rec->unk3A, 2, rec) != 0)
    {
        if ((rec->unk21 & ~0x80) == 0x34)
        {
            SET_ANIM(rec->unk3A, 0x50, 0x26, 0);
            rec->unk2A = 0xB85;
            func_8008E690(rec);
            func_800A2DD8(rec->unk3A);
        }
        if ((rec->unk21 & ~0x80) == 0x35)
        {
            if ((D_80105AE0[rec->unk3A].unk178 >> 1) & 1)
            {
                D_80105AE0[D_80105AE0[rec->unk3A].unk170].unkC &= ~0x2000;
                rec->unk2A = 0;
                func_80096334(rec);
                tmp = func_800839F8(rec->unk3A, 0);
                if (tmp != -1)
                {
                    if (func_8008AABC(rec->unk3A, D_80105AE0[rec->unk3A].unk170) != 0)
                    {
                        if (D_800FD818[rec->unk3A].unk1 == 8)
                        {
                            func_8008A9D8(rec->unk3A, D_80105AE0[rec->unk3A].unk170, 0xD);
                        }
                        else
                        {
                            func_8008A9D8(rec->unk3A, D_80105AE0[rec->unk3A].unk170, 0xC);
                        }
                        anim_id = 0x64;
                        if (D_800FD818[rec->unk3A].unk1 == 8)
                        {
                            anim_id = 0x61;
                        }
                    }
                    else
                    {
                        if (D_800FD818[rec->unk3A].unk1 == 8)
                        {
                            func_8008A9D8(D_80105AE0[rec->unk3A].unk170, rec->unk3A, 0x18);
                        }
                        else
                        {
                            func_8008A9D8(D_80105AE0[rec->unk3A].unk170, rec->unk3A, 0x17);
                        }
                        anim_id = 0x65;
                        if (D_800FD818[rec->unk3A].unk1 == 8)
                        {
                            anim_id = 0x63;
                        }
                    }
                    if (func_80083EEC(rec->unk3A, tmp, anim_id) != 0)
                    {
                        targets = D_80105AE0[rec->unk3A].unk170;
                        field_start_actor_animation(tmp, 1, &targets);
                    }
                }
            }
            func_800A2DD8(rec->unk3A);
        }
    }
}
