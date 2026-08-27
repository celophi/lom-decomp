#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
} UnkStruct14;

void func_800832F0(UnkStruct14 *arg0, UnkStruct14 *arg1)
{
    arg0->unk0 = arg1->unk0;
    arg0->unk4 = arg1->unk4;
    arg0->unk8 = arg1->unk8;
    arg0->unkC = arg1->unkC;
    arg0->unk10 = arg1->unk10;
}

typedef struct
{
    u32 unk0;         /* 0x00 */
    s32 unk4;         /* 0x04 */
    u8  unk8[3];      /* 0x08 */
    u8  unkB;         /* 0x0B */
    u8  unkC[6];      /* 0x0C */
    u8  unk12;        /* 0x12 */
    u8  unk13[0x35];  /* 0x13, stride 0x48 */
} UnkPartEntry;

/**
 * @see decomp.me (100%) local match - no scratch link created.
 */
void func_8008332C(u8 *arg0, UnkPartEntry *arg1, s32 arg2)
{
    s32 var_s1;
    u32 temp_a3;
    u8 temp_v0_2;
    u8 temp_test;
    s32 temp_a0;

    for (var_s1 = 0; var_s1 < arg2; var_s1++)
    {
        temp_test = (arg1[var_s1].unkB + 9) & 0xFF;
        if ((u32) temp_test < 3U)
        {
            if (arg1[var_s1].unk4 & 1)
            {
                temp_a3 = arg1[var_s1].unk0;
                func_8008343C(0xF9 - arg1[var_s1].unkB, temp_a3 & 3, arg0, ((temp_a3 >> 8) & 7) + 1);
            }
            if (((u32) arg1[var_s1].unk0 >> 0x16) & 1)
            {
                if ((arg1[var_s1].unk12 != 0) && (field_get_track_counter_modulo(arg0, arg1[var_s1].unk12) == 0))
                {
                    temp_a0 = 0xF9 - arg1[var_s1].unkB;
                    temp_v0_2 = (arg0 + var_s1)[0x2B] + 1;
                    (arg0 + var_s1)[0x2B] = temp_v0_2;
                    func_80083868(temp_a0, temp_v0_2 & 0xFF, arg0);
                }
            }
        }
    }
}

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

typedef struct
{
    u8 unk0;
    u8 unk1;
    u8 unk2;   /* width */
    u8 unk3;   /* height */
    u16 *unk4; /* pixel data */
} PartEntry;   /* 0x8 */

typedef struct
{
    u8 pad0[2];
    u8 unk2;          /* part count */
    u8 pad3;
    PartEntry *unk4;  /* parts array */
    u8 pad8[0x18 - 0x8];
} ActorEntry;         /* 0x18 */

typedef struct
{
    u8 pad0[0x18];
    ActorEntry *unk18;
    u8 pad1C[0x228 - 0x1C];
    u8 unk228;
} FieldCtx;

/**
 * @brief Blit each part of an actor sprite into the PSX scratchpad with one of
 *        four rotation/flip modes, then LoadImage it into VRAM.
 * @param arg0 Actor index into arg2->unk18[].
 * @param arg1 Rotation mode selector (0..3).
 * @param arg2 Field context; unk18 is the actor array, unk228 selects the VRAM
 *             destination page.
 * @param arg3 Split offset (rows/columns rotated to the far edge).
 * @note WIP 96.58%. The residual is register-allocation only (no structural
 *       diff): (1) a saved-register swap - the target puts arg0 in s5 and arg2
 *       in s6, this compile has them exchanged, cascading through var_s2/arg3*2
 *       and the case 2/3 scratchpad-walker temps; (2) eight `addiu ..., +2`
 *       scratchpad-pointer increments that splat over-symbolizes as
 *       %lo(D_1F800002). Those eight are byte-identical to the retail bytes and
 *       clear once field_reloc_addrs.txt gains MIPS_NONE entries at rom offsets
 *       0x338B1, 0x33909, 0x33971, 0x339C9, 0x33A15, 0x33A55, 0x33ACD, 0x33B0D
 *       and the overlay is re-splatted; real match is ~99%.
 * @see decomp.me (96.58%) local match - no scratch link created.
 */
void func_8008343C(s32 arg0, s32 arg1, FieldCtx *arg2, s32 arg3)
{
    RECT rect;
    ActorEntry *actor;
    PartEntry *part;
    u16 *scratch;
    u16 *sp_w;
    u16 *src;
    u16 *dst;
    s32 n;
    u16 temp;
    u8 width;
    u8 rows;
    s32 var_s2;

    scratch = (u16 *)0x1F800000;
    var_s2 = 0;
    actor = &arg2->unk18[arg0];
    if (actor->unk2 != 0)
    {
        do
        {
            part = &actor->unk4[var_s2];
            switch (arg1)
            {
            case 0:
                src = part->unk4;
                n = part->unk2 * arg3;
                dst = src;
                if (n != 0)
                {
                    do
                    {
                        n -= 1;
                        temp = *src;
                        src += 1;
                        *scratch = temp;
                        scratch += 1;
                    } while (n != 0);
                }
                n = part->unk2 * (part->unk3 - arg3);
                if (n != 0)
                {
                    do
                    {
                        n -= 1;
                        temp = *src;
                        src += 1;
                        *dst = temp;
                        dst += 1;
                    } while (n != 0);
                }
                n = part->unk2 * arg3;
                scratch = (u16 *)0x1F800000;
                if (n != 0)
                {
                    do
                    {
                        n -= 1;
                        temp = *scratch;
                        scratch += 1;
                        *dst = temp;
                        dst += 1;
                    } while (n != 0);
                }
                break;
            case 1:
                width = part->unk2;
                src = &part->unk4[width * part->unk3] - 1;
                n = width * arg3;
                dst = src;
                if (n != 0)
                {
                    do
                    {
                        n -= 1;
                        temp = *src;
                        src -= 1;
                        *scratch = temp;
                        scratch += 1;
                    } while (n != 0);
                }
                n = part->unk2 * (part->unk3 - arg3);
                if (n != 0)
                {
                    do
                    {
                        n -= 1;
                        temp = *src;
                        src -= 1;
                        *dst = temp;
                        dst -= 1;
                    } while (n != 0);
                }
                n = part->unk2 * arg3;
                scratch = (u16 *)0x1F800000;
                if (n != 0)
                {
                    do
                    {
                        n -= 1;
                        temp = *scratch;
                        scratch += 1;
                        *dst = temp;
                        dst -= 1;
                    } while (n != 0);
                }
                break;
            case 2:
                rows = part->unk3;
                dst = part->unk4;
                if (rows != 0)
                {
                    do
                    {
                        n = 0;
                        if (arg3 != 0)
                        {
                            sp_w = scratch;
                            src = dst;
                            do
                            {
                                temp = *src;
                                src += 1;
                                n += 1;
                                *sp_w = temp;
                                sp_w += 1;
                            } while (n != arg3);
                        }
                        n = part->unk2 - arg3;
                        if (n != 0)
                        {
                            do
                            {
                                n -= 1;
                                *dst = dst[arg3];
                                dst += 1;
                            } while (n != 0);
                        }
                        n = 0;
                        if (arg3 != 0)
                        {
                            sp_w = scratch;
                            do
                            {
                                temp = *sp_w;
                                sp_w += 1;
                                n += 1;
                                *dst = temp;
                                dst += 1;
                            } while (n != arg3);
                        }
                        rows -= 1;
                    } while (rows != 0);
                }
                break;
            case 3:
                dst = &part->unk4[part->unk2 * part->unk3] - 1;
                rows = part->unk3;
                if (rows != 0)
                {
                    do
                    {
                        n = 0;
                        if (arg3 != 0)
                        {
                            sp_w = scratch;
                            src = dst;
                            do
                            {
                                temp = *src;
                                src -= 1;
                                n += 1;
                                *sp_w = temp;
                                sp_w += 1;
                            } while (n != arg3);
                        }
                        n = part->unk2 - arg3;
                        if (n != 0)
                        {
                            do
                            {
                                n -= 1;
                                *dst = *(dst - arg3);
                                dst -= 1;
                            } while (n != 0);
                        }
                        n = 0;
                        if (arg3 != 0)
                        {
                            sp_w = scratch;
                            do
                            {
                                temp = *sp_w;
                                sp_w += 1;
                                n += 1;
                                *dst = temp;
                                dst -= 1;
                            } while (n != arg3);
                        }
                        rows -= 1;
                    } while (rows != 0);
                }
                break;
            }
            if ((u8) arg2->unk228 < 2U)
            {
                rect.x = (arg2->unk228 << 6) + (part->unk0 + 0x340);
                rect.y = part->unk1 + 0x100;
            }
            else
            {
                rect.x = part->unk0 + 0x140;
                rect.y = part->unk1;
            }
            rect.w = part->unk2;
            rect.h = part->unk3;
            LoadImage(&rect, part->unk4);
            actor = &arg2->unk18[arg0];
            var_s2 += 1;
        } while (var_s2 < (s32) actor->unk2);
    }
}

/**
 * @brief Reload one part of an actor sprite (selected by arg1 modulo the part
 *        count) into VRAM via LoadImage, using the base part's RECT dimensions.
 * @param arg0 Actor index into arg2->unk18[].
 * @param arg1 Part selector; the part used is arg1 % actor->unk2.
 * @param arg2 Field context; unk18 is the actor array, unk228 selects the VRAM
 *             destination page.
 * @see decomp.me (100%) local match - no scratch link created.
 */
void func_80083868(s32 arg0, s32 arg1, FieldCtx *arg2)
{
    RECT rect;
    PartEntry *base;
    PartEntry *part;

    base = arg2->unk18[arg0].unk4;
    part = &base[arg1 % (s32) arg2->unk18[arg0].unk2];
    if ((u8) arg2->unk228 < 2U)
    {
        rect.x = (arg2->unk228 << 6) + (base->unk0 + 0x340);
        rect.y = base->unk1 + 0x100;
    }
    else
    {
        rect.x = base->unk0 + 0x140;
        rect.y = base->unk1;
    }
    rect.w = base->unk2;
    rect.h = base->unk3;
    LoadImage(&rect, part->unk4);
}
