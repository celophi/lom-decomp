#include "common.h"

typedef struct
{
    u8* start; /* 0x00 */
    u8* end;   /* 0x04 */
    u8 unk8;   /* 0x08 */
    u8 slot_index; /* 0x09 */
    u8 padA[0xE - 0xA];
    s16 unkE; /* 0x0E */
    u32 flags; /* 0x10 */
} FieldResourceEntry;

extern FieldResourceEntry g_field_resource_entries[];

/**
 * @brief Find the resource entry slot matching arg0, or the first free slot
 *        if none matches, then hand it off to func_8006B240.
 * @param arg0 Resource slot identifier to search for.
 * @param arg1 Passed through unchanged to func_8006B240.
 * @see decomp.me (100%) TODO
 */
void func_8006B1A0(s32 arg0, s32 arg1)
{
    s32 i;

    for (i = 0; i < 9; i++)
    {
        if (((g_field_resource_entries[i].flags >> 1) & 1) && g_field_resource_entries[i].slot_index == arg0)
        {
            break;
        }
    }

    if (i == 9)
    {
        for (i = 0; i < 9; i++)
        {
            if (!((g_field_resource_entries[i].flags >> 1) & 1))
            {
                break;
            }
        }
    }

    func_8006B240(arg0, arg1, i);
}

typedef struct
{
    s32 unk0; /* 0x00 */
    s32 unk4; /* 0x04 */
    s32 unk8; /* 0x08 */
    u32 unkC; /* 0x0C */
    s16 unk10; /* 0x10 */
    s16 unk12; /* 0x12 */
    s16 unk14; /* 0x14 */
    s16 unk16; /* 0x16 */
    u8 unk18; /* 0x18 */
    u8 unk19; /* 0x19 */
    u8 unk1A; /* 0x1A */
    u8 pad1B[0x1C - 0x1B];
    s32 unk1C; /* 0x1C */
    u8 pad20[0x21 - 0x20];
    u8 unk21; /* 0x21 */
    u8 unk22; /* 0x22 */
    u8 unk23; /* 0x23 */
    u8 unk24; /* 0x24 */
    u8 unk25; /* 0x25 */
    u8 pad26[0x27 - 0x26];
    u8 unk27; /* 0x27 */
    u8 unk28; /* 0x28 */
    u8 pad29[0x2A - 0x29];
    s16 unk2A; /* 0x2A */
    s16 unk2C; /* 0x2C */
    u16 unk2E; /* 0x2E */
    s16 unk30; /* 0x30 */
    u8 unk32; /* 0x32 */
    u8 unk33; /* 0x33 */
    u8 unk34; /* 0x34 */
    u8 unk35; /* 0x35 */
    u8 unk36; /* 0x36 */
    u8 unk37; /* 0x37 */
    u8 unk38; /* 0x38 */
    u8 pad39[0x3A - 0x39];
    u8 unk3A; /* 0x3A */
    u8 unk3B; /* 0x3B */
    u32 unk3C; /* 0x3C */
    s32 unk40; /* 0x40 */
    u32 unk44; /* 0x44 */
    u32 unk48; /* 0x48 */
    u32 unk4C; /* 0x4C */
    u8 pad50[0x54 - 0x50];
} Struct_D800FDF58;

typedef struct
{
    u8 pad0[0xC];
    u32 unkC; /* 0x0C */
    u32 unk10; /* 0x10 */
    u8 pad14[0x12C - 0x14];
    u32 unk12C; /* 0x12C */
    u8 pad130[0x140 - 0x130];
    s16 unk140; /* 0x140 */
    s16 unk142; /* 0x142 */
    s16 unk144; /* 0x144 */
    s16 unk146; /* 0x146 */
    u8 pad148[0x178 - 0x148];
    s32 unk178; /* 0x178 */
    u8 pad17C[0x18E - 0x17C];
    u8 unk18E; /* 0x18E */
    u8 pad18F[0x19C - 0x18F];
    s32 unk19C; /* 0x19C */
    s32 unk1A0; /* 0x1A0 */
    u8 pad1A4[0x1A8 - 0x1A4];
    u8 unk1A8; /* 0x1A8 */
    u8 unk1A9; /* 0x1A9 */
    u8 unk1AA; /* 0x1AA */
    u8 pad1AB[0x23C - 0x1AB];
} Struct_D80105AE0;

extern Struct_D800FDF58 D_800FDF58[];
extern Struct_D80105AE0 D_80105AE0[];
extern void* g_field_resource_cursor;
extern u8 D_800FDA83;
extern u8 D_800FDCEA;
extern u8 *g_pad_ctx;
extern s32 D_800F229C;
extern s32 D_800FE754;
extern s32 D_80122710;
extern s32 D_80122714;
extern s32 D_80122B20;
extern s32 g_field_return_to_title_prompt_state;
extern Struct_D800FDF58 D_800FF658[];
extern s32 D_80105770;
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;

typedef struct
{
    s16 x;
    s16 y;
} Vec2s;

/**
 * @brief Initialize resource entry arg2 for slot arg0, then notify every
 *        D_800FDF58 record that references it via func_8006C3FC.
 * @param arg0 Resource slot identifier, stored into the entry.
 * @param arg1 Base pointer; func_8006CAFC's first argument is arg1 + 0xB52.
 * @param arg2 Resource entry index to (re)initialize.
 * @see decomp.me (100%) TODO
 */
void func_8006B240(s32 arg0, u8 *arg1, s32 arg2)
{
    s32 i;
    FieldResourceEntry *entry;
    FieldResourceEntry *base;

    func_8006B354(arg2);

    base = g_field_resource_entries;
    entry = &base[arg2];
    entry->slot_index = arg0;
    entry->unk8 = 0;
    func_8009C434();
    entry->unkE = 0;
    entry->flags &= ~1;
    entry->start = g_field_resource_cursor;
    func_8006CAFC(arg1 + 0xB52, arg0, arg2);
    entry->end = g_field_resource_cursor;
    entry->flags |= 2;

    {
        Struct_D800FDF58 *rec;
        u8 *info;

        rec = D_800FDF58;
        info = (u8*)D_800FDF58 + 0x3B;
        i = 0;
        while (i < 0xD)
        {
            if (info[-0x16] != 0xFF && info[0] == arg2)
            {
                func_8006C3FC(rec);
            }
            i++;
            info += 0x54;
            rec++;
        }
    }
}

/**
 * @brief If resource entry arg0 is in use, compact its buffer region out of
 *        g_field_resource_cursor's arena and shift every later entry and
 *        D_800FDF58 record down by the freed size.
 * @param arg0 Resource entry index to release.
 * @see decomp.me (98.89%) TODO
 * @note Residual: a 1-insn compiler-temp coloring gap around the `dst`
 *       load right after the entry guard branch (target loads it into v0
 *       then copies to a1; ours loads directly into a1). Possibly the same
 *       class as the sibling func_8006AB38 (src/overlays/field/field4.c,
 *       97.74%, see working/func_8006AB38/STATUS.md), though that function
 *       has not been re-checked under the correct toolchain. No source-shape
 *       change or permuter run closed it here.
 */
void func_8006B354(s32 arg0)
{
    s32 i;
    u8 *src;
    u8 *dst;
    s32 size;
    Struct_D800FDF58 *p;

    if ((g_field_resource_entries[arg0].flags >> 1) & 1)
    {
        dst = g_field_resource_entries[arg0].start;
        src = g_field_resource_entries[arg0].end;
        size = src - dst;

        while (src != g_field_resource_cursor)
        {
            *dst = *src;
            src++;
            dst++;
        }

        for (i = 0; i < 8; i++)
        {
            if (((g_field_resource_entries[i].flags >> 1) & 1) && g_field_resource_entries[i].start > g_field_resource_entries[arg0].start)
            {
                g_field_resource_entries[i].start -= size;
                g_field_resource_entries[i].end -= size;
            }
        }

        p = D_800FDF58;
        for (i = 0; i < 0xD; i++)
        {
            if ((p->unk25 != 0xFF) && (p->unk3B != 8))
            {
                if ((p->unk40 | 0x80000000) > (((u32)g_field_resource_entries[arg0].start) | 0x80000000))
                {
                    p->unk40 -= size;
                }
            }
            p++;
        }

        g_field_resource_cursor = ((u8*)g_field_resource_cursor) - size;
    }
}

/**
 * @brief Reset D_800FDF58/D_80105AE0 entry arg0 to its default state for a
 *        newly-created field object tied to resource slot arg1, applying a
 *        few slot-specific special cases (slot 1's palette flag, slot 2's
 *        pad-timer flag).
 * @param arg0 D_800FDF58/D_80105AE0 entry index to (re)initialize.
 * @param arg1 Resource slot index whose g_field_resource_entries slot_index
 *        is copied into the new entry's unkC.
 * @see decomp.me (86.64%) TODO
 * @note NOT MATCHED. Required to match, each measured by reverting it:
 *       - the loop counter i must be u32, not s32 (+7 rows) - otherwise gcc
 *         reverses the zero-fill loop into a down-counting form the target
 *         does not use;
 *       - the three 0xFFFF7FFF/0xEFFFFFFF/0xFFFBFFFF masks are separate
 *         locals assigned right after the zero-fill loop, not inline
 *         literals at point of use (+24 rows) - the target computes them
 *         early;
 *       - the flags &= ~0x1FF and flags |= 2 steps are two statements, not
 *         one combined expression (+26 rows);
 *       - the arg0==1 and arg0==2 special cases use an explicit
 *         Struct_D800FDF58* pointer for the constant-indexed entry (+17
 *         rows), not D_800FDF58[1]/[2] directly;
 *       - arg1 is reused as a scratch holding 0x80 for unk1A/unk19/unk18
 *         instead of the literal (structural fix, argdiff/target-only both
 *         drop).
 *       Residue (13 target-only / 8 yours-only rows) is register coloring
 *       around repeated D_800FDF58[arg0] base-address recomputation and
 *       the mask constants; a permuter run (9000+ iterations) found a
 *       `new_var = arg0` copy-and-reuse shape that scores better but did
 *       not reproduce cleanly by hand. See working/func_8006B4D0/ for the
 *       current source.
 */
void func_8006B4D0(s32 arg0, s32 arg1)
{
    u32 i;
    u8 *p;
    s32 flags;
    s32 mask_a;
    s32 mask_b;
    s32 mask_c;

    p = (u8*)&D_800FDF58[arg0];
    i = 0;
    do
    {
        *p = 0;
        i++;
        p++;
    } while (i < 0x54);

    mask_a = 0xFFFF7FFF;
    mask_b = 0xEFFFFFFF;
    mask_c = 0xFFFBFFFF;

    D_80105AE0[arg0].unk19C = -1;
    flags = D_80105AE0[arg0].unk178;
    D_80105AE0[arg0].unk1A0 = 0;
    D_80105AE0[arg0].unk18E = 0;
    flags &= ~0x80;
    flags &= ~0x40;
    D_80105AE0[arg0].unk178 = flags;

    D_800FDF58[arg0].unk22 = (s8)(arg0 + 0x30);
    D_800FDF58[arg0].unk28 = 0xFF;
    flags = D_800FDF58[arg0].unk1C;
    D_800FDF58[arg0].unk3A = arg0;
    D_800FDF58[arg0].unk24 = 0;
    D_800FDF58[arg0].unk25 = 0;
    D_800FDF58[arg0].unk27 = 0;
    D_800FDF58[arg0].unk2A = 0;
    D_800FDF58[arg0].unk2C = 0;
    D_800FDF58[arg0].unk2E = 0;
    D_800FDF58[arg0].unk30 = 0;
    D_800FDF58[arg0].unk32 = 0;
    D_800FDF58[arg0].unk33 = 0;
    D_800FDF58[arg0].unk4 = 0;
    D_800FDF58[arg0].unk8 = 0;
    flags = flags & ~0x1FF;
    flags = flags | 2;
    D_800FDF58[arg0].unk1C = flags;
    D_800FDF58[arg0].unk0 = 0xFFFB0000;
    D_800FDF58[arg0].unk1C = flags & mask_a & mask_b & mask_c;

    if (arg0 == 1 && D_800FDA83 == 0)
    {
        Struct_D800FDF58 *entry1 = &D_800FDF58[1];
        entry1->unk1C = (entry1->unk1C & 0xFF87FFFF) | 0x500000;
    }
    else
    {
        D_800FDF58[arg0].unk1C &= 0xFF87FFFF;
    }

    D_800FDF58[arg0].unk1C &= ~0x800;
    D_800FDF58[arg0].unk3B = arg1;
    D_800FDF58[arg0].unk21 = 0;
    D_800FDF58[arg0].unkC = g_field_resource_entries[arg1].slot_index;
    D_800FDF58[arg0].unk10 = 0;
    D_800FDF58[arg0].unk12 = 0;
    D_800FDF58[arg0].unk14 = 0;
    D_800FDF58[arg0].unk16 = 1;
    D_800FDF58[arg0].unk34 = 0;
    D_800FDF58[arg0].unk44 = 0;
    D_800FDF58[arg0].unk48 = 0;
    D_800FDF58[arg0].unk4C = 0;
    arg1 = 0x80;
    D_800FDF58[arg0].unk1A = arg1;
    D_800FDF58[arg0].unk19 = arg1;
    D_800FDF58[arg0].unk18 = arg1;

    if (arg0 == 2 && D_800FDCEA >= 0x41)
    {
        Struct_D800FDF58 *entry2 = &D_800FDF58[2];
        entry2->unk1C = (entry2->unk1C & 0xFFFCFFFF) | (((g_pad_ctx[0x29D7] + 1) & 3) << 16);
        return;
    }

    D_800FDF58[arg0].unk1C &= 0xFFFCFFFF;
}

typedef struct
{
    u32 unk0; /* 0x00 */
    u32 unk4; /* 0x04 */
    u8 unk8; /* 0x08 */
    u8 unk9; /* 0x09 */
    u8 padA;
    u8 unkB; /* 0x0B */
    u8 unkC; /* 0x0C */
    u8 unkD; /* 0x0D */
    u8 unkE; /* 0x0E */
    u8 unkF; /* 0x0F */
    u8 unk10; /* 0x10 */
    u8 unk11; /* 0x11 */
    u8 pad12[0x14 - 0x12];
    u32 unk14; /* 0x14 (overlaps unk16 at its upper halfword) */
    s16 unk18; /* 0x18 */
    u8 pad1A[0x23 - 0x1A];
    u8 unk23; /* 0x23 */
    u32 unk24; /* 0x24 (overlaps byte writes at 0x24/0x25) */
    u32 unk28; /* 0x28 */
    u8 unk2C; /* 0x2C */
    u8 pad2D;
    u8 unk2E; /* 0x2E */
    u8 pad2F[0x31 - 0x2F];
    u8 unk31; /* 0x31 */
    u8 pad32;
    u8 unk33; /* 0x33 */
    u32 unk34; /* 0x34 */
    u8 pad38[0x48 - 0x38];
} FieldActorPartDef;

extern FieldActorPartDef D_800FE3A0[];

typedef struct FieldActorAnimationDef
{
    u8 unk0[2];
    u8 pad2[0xC - 2];
    u16 unkC;
    u16 unkE;
    u16 unk10;
    u8 pad10[0x14 - 0x12];
    u8 unk14;
    u8 unk15;
    u8 pad16[0x18 - 0x16];
    u16 unk18;
} FieldActorAnimationDef;

typedef struct
{
    FieldActorPartDef* unk0;
    u8 pad4[0xC - 4];
    FieldActorAnimationDef* unkC;
    u8 pad10[0x24 - 0x10];
    u8 unk24;
    u8 unk25;
    u8 unk26;
    u8 unk27;
    u8 unk28;
    u8 unk29;
    u8 unk2A;
    u8 unk2B[16];
    u8 unk3B[9][16];
    u8 padCB;
    u16 unkCC[9][16];
    u16 unk1EC[9];
    u8 pad1FE[0x222 - 0x1FE];
    u16 unk222;
    union
    {
        u32 unk224;
        struct
        {
            u16 lo;
            u16 animation_id;
        } h;
    } u224;
    u8 owner_object_index;
    u8 unk229[9];
    u8 unk232;
    u8 unk233;
    u16 unk234;
    u16 unk236;
    u8 pad238[2];
    u8 unk23A;
    u8 unk23B;
    u8 pad23C[0x240 - 0x23C];
    u16* unk240;
} FieldActorState;

extern FieldActorState g_field_actor_slots[];
extern s32 D_800F2298;
extern s32 D_8012269C;
extern s32 D_801227C8;
extern s32 g_field_track_index;
extern s32 D_80105760;

/**
 * @brief Zero-fill D_800FE3A0 entry arg0, then set it up as a default field
 *        actor part: fade timers, kind 8, a size/idle-timer sized by arg1,
 *        and a couple of render/state flag bits.
 * @param arg0 D_800FE3A0 entry index to (re)initialize.
 * @param arg1 Selects the unk2E/unk33 timer value (0x40 if zero, else 0x30).
 * @see decomp.me (73.82%) TODO
 * @note NOT MATCHED. Required to match, each measured:
 *       - the loop counter and the flags-related mask both benefit from an
 *         early-hoisted constant (`i = 0xFF;` before the unk2E/unk33 writes,
 *         and splitting `(unk14 & ~0xF0) | 0x20` into two statements) - both
 *         proven via probe_variants (+7 and +1 rows respectively).
 *       Residue (13 target-only / 5 yours-only rows) is the target
 *       RE-COMPUTING `&D_800FE3A0[arg0]` a third time for the final field
 *       writes (unk11/unk14/unk28/unk24), where our compile reuses the entry
 *       pointer from region 2. Root cause traced this session: the target's
 *       `val` if/else compiles to the two-arm form (beqz; then `j`; else),
 *       whose multi-predecessor join label is a cse.c basic-block boundary -
 *       that reset is what forces the recompute (idioms.md [CSE-09]). Ours
 *       instead compiles to the PRELOAD form (val=0x40 hoisted, single
 *       branch), because arg0 dies into the region-2 entry pointer ($a2),
 *       leaving $a3 free for val; with no join label, cse never resets and
 *       folds region 3. So the real lever is forcing the two-arm branch /
 *       keeping arg0 live in $a3 - NOT a local address rewrite. Measured
 *       inert this session: default+override val form, a separate entry2
 *       pointer, splitting the unk14 expression, and a do/while(0) barrier
 *       (the last only shuffles a store, +1 row of scheduling noise). All
 *       seven toolchains tried; the four gcc 2.7.2/2.8.0 ones tie at 73.82%
 *       with this identical gap. A permuter run (70000+ iterations) found
 *       nothing beyond the two hoists above.
 */
void func_8006B7A0(s32 arg0, s32 arg1)
{
    s32 i;
    u32 *p;
    FieldActorPartDef *entry;
    s32 val;

    p = (u32*)&D_800FE3A0[arg0];
    i = 0x12;
    do
    {
        *p = 0;
        i--;
        p++;
    } while (i != 0);

    entry = &D_800FE3A0[arg0];
    entry->unk10 = 0x80;
    entry->unkF = 0x80;
    entry->unkE = 0x80;
    entry->unk8 = 1;
    entry->unk9 = 0xFF;
    *(s16*)((u8*)entry + 0x16) = 0x14;
    entry->unkD = 8;
    *((u8*)entry + 0x25) = 8;
    *((u8*)entry + 0x24) = 8;
    entry->unk23 = 8;
    entry->unk18 = 0x100;
    entry->unk4 = (entry->unk4 | 0x800) & 0xFF3FFFFF;
    entry->unk4 |= 0x400000;
    entry->unk0 = entry->unk0 & 0xFCFFFFFF;
    entry->unk0 |= 0x2000000;

    if (arg1 != 0)
    {
        val = 0x30;
    }
    else
    {
        val = 0x40;
    }

    i = 0xFF;
    entry->unk2E = val;
    entry->unk33 = val;

    entry = &D_800FE3A0[arg0];
    entry->unk11 = i;
    val = entry->unk14 & ~0xF0;
    entry->unk14 = val | 0x20;
    entry->unk28 |= 0x2000000;
    entry->unk24 |= 0x100000;
}

/**
 * @brief Broadcast a shared render/config state to every field actor slot:
 *        stores arg0-arg2 into all 13 D_80105AE0 and D_800FE3A0 entries, folds
 *        arg4 into the 2-bit field at bits 22-23 of each part's unk4, and folds
 *        arg3's low bit into bit 23 of each D_800FDF58 record's unk1C.
 * @param arg0 Byte written to unk1A8 (D_80105AE0) and unkE (D_800FE3A0).
 * @param arg1 Byte written to unk1A9 (D_80105AE0) and unkF (D_800FE3A0).
 * @param arg2 Byte written to unk1AA (D_80105AE0) and unk10 (D_800FE3A0).
 * @param arg3 Low bit replaces bit 23 of every D_800FDF58 entry's unk1C.
 * @param arg4 Low 2 bits replace bits 22-23 of every D_800FE3A0 entry's unk4.
 * @see decomp.me (100%) TODO
 */
void func_8006B8DC(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    s32 i;
    u32 mode;
    u32 flag;

    mode = (arg4 & 3) << 22;
    for (i = 0; i < 13; i++)
    {
        D_80105AE0[i].unk1A8 = arg0;
        D_80105AE0[i].unk1A9 = arg1;
        D_80105AE0[i].unk1AA = arg2;
        D_800FE3A0[i].unkE = arg0;
        D_800FE3A0[i].unkF = arg1;
        D_800FE3A0[i].unk10 = arg2;
        D_800FE3A0[i].unk4 = (D_800FE3A0[i].unk4 & 0xFF3FFFFF) | mode;
    }

    for (i = 0; i < 13; i++)
    {
        flag = (arg3 & 1) << 23;
        D_800FDF58[i].unk1C = (D_800FDF58[i].unk1C & 0xFF7FFFFF) | flag;
    }
}

Struct_D800FDF58* func_80087C9C(s32);

/**
 * @brief Apply a shared render/config state to the single field actor slot that
 *        owns record arg5: stores arg0-arg2 into that slot's D_80105AE0 and
 *        D_800FE3A0 entries, folds arg4 into the 2-bit field at bits 22-23 of
 *        the part's unk4, and arg3's low bit into bit 23 of the record's unk1C.
 * @param arg0 Byte written to unk1A8 (D_80105AE0) and unkE (D_800FE3A0).
 * @param arg1 Byte written to unk1A9 (D_80105AE0) and unkF (D_800FE3A0).
 * @param arg2 Byte written to unk1AA (D_80105AE0) and unk10 (D_800FE3A0).
 * @param arg3 Low bit replaces bit 23 of the record's unk1C.
 * @param arg4 Low 2 bits replace bits 22-23 of the part's unk4.
 * @param arg5 Record selector passed to func_80087C9C.
 * @return 0 on success, -1 if func_80087C9C found no record.
 * @see decomp.me (100%) TODO
 */
s32 func_8006B984(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    Struct_D800FDF58 *rec;

    rec = func_80087C9C(arg5);
    if (rec == (Struct_D800FDF58*)-1)
    {
        return -1;
    }

    D_80105AE0[rec->unk3A].unk1A8 = arg0;
    D_80105AE0[rec->unk3A].unk1A9 = arg1;
    D_80105AE0[rec->unk3A].unk1AA = arg2;
    D_800FE3A0[rec->unk3A].unkE = arg0;
    D_800FE3A0[rec->unk3A].unkF = arg1;
    D_800FE3A0[rec->unk3A].unk10 = arg2;
    D_800FE3A0[rec->unk3A].unk4 = (D_800FE3A0[rec->unk3A].unk4 & 0xFF3FFFFF) | ((arg4 & 3) << 22);
    rec->unk1C = (rec->unk1C & 0xFF7FFFFF) | ((arg3 & 1) << 23);
    return 0;
}

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

/** @brief Staging buffer that field CD reads land in. */
typedef struct
{
    u32 unk0;  /* 0x00 */
    u32 unk4;  /* 0x04 */
    s32 unk8;  /* 0x08 byte offset of the second image inside the data area */
    u32 unkC;  /* 0x0C */
    u16 unk10; /* 0x10 */
    u16 unk12; /* 0x12 */
    u16 unk14; /* 0x14 start of the image data */
} FieldCdBuffer;

extern FieldCdBuffer* D_8010D038;

/*
 * Per-element structure (stride 0x268). D_800FD818 is a 3-element array; the
 * absolute offsets previously used by func_8006A324 (0x268, 0x4BC, 0x4D0,
 * 0x724, ...) are elements [1] and [2] of this array.
 */
typedef struct
{
    union
    {
        u16 h; /* offset 0x00 as a halfword (flags) */
        struct
        {
            u8 unk0; /* offset 0x00 */
            u8 unk1; /* offset 0x01 */
        } b;
    } u0;
    u8 unk2;                /* offset 0x02 */
    u8 unk3;                /* offset 0x03 */
    u8 pad0[0x254 - 4];     /* 0x04 .. 0x253 */
    u16 unk254;             /* offset 0x254 */
    u8 unk256;              /* offset 0x256 */
    u8 pad1[0x268 - 0x257]; /* 0x257 .. 0x267 */
} D_800FD818_type;

extern D_800FD818_type D_800FD818[];
extern u8 g_prim_rect_buf[];

/**
 * @brief Read a field texture set off the CD and upload it to VRAM: one
 *        optional full/partial strip at y = arg4 + 0x1F4, then the tile block
 *        for the current slot.
 * @param arg0 CD resource index (masked to 16 bits) to queue.
 * @param arg1 Slot index; < 2 selects the 0x380-based tile column.
 * @param arg2 Column index scaled by 0x40 into the tile block's x.
 * @param arg3 Selects the narrow (0x40-wide / 0x80-tall) variants.
 * @param arg4 Base row added to 0x1F4 for the first upload's y.
 * @see decomp.me (100%) TODO
 */
void func_8006BAF8(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    RECT rect;
    FieldCdBuffer* buf;
    s32 second;
    s32 full_width;

    buf = D_8010D038;
    full_width = 0x100;
    cdrom_queue_read(arg0 & 0xFFFF, buf);
    cdrom_wait_queue_empty();
    second = buf->unk8;
    buf->unk14 = 0;

    if (arg2 == 0 || arg3 != 0)
    {
        if (arg3 != 0)
        {
            rect.x = 0xC0;
            rect.y = arg4 + 0x1F4;
            rect.w = 0x40;
            rect.h = 1;
        }
        else
        {
            rect.y = arg4 + 0x1F4;
            rect.w = full_width;
            rect.x = 0;
            rect.h = 1;
        }
        LoadImage(&rect, &buf->unk14);
    }

    if (arg1 >= 2)
    {
        s32 base = (arg2 << 6) + 0x340;
        s32 off = arg1 << 6;
        rect.x = base - off;
        rect.w = 0x40;
        rect.y = 0;
        rect.h = 0x100;
    }
    else if (arg3 != 0)
    {
        s32 base = (arg2 << 6) + 0x380;
        rect.x = base - (arg1 << 7);
        rect.y = 0x80;
        rect.w = 0x40;
        rect.h = 0x80;
    }
    else
    {
        s32 base = (arg2 << 6) + 0x380;
        s32 off = arg1 << 7;
        rect.x = base - off;
        rect.w = 0x40;
        rect.y = 0;
        rect.h = 0x100;
    }

    LoadImage(&rect, (u8*)(second + (s32)buf + 0x14));
    DrawSync(0);
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006BC50(void)
{
    Struct_D800FDF58 *rec;
    Struct_D80105AE0 *ent;
    s32 i;
    s32 mode;
    s32 count;
    s32 index;

    rec = &D_800FDF58[0];
    ent = &D_80105AE0[0];

    if (D_80122714 == 0)
    {
        func_8009184C();
        if (D_80122714 == 0 && D_800FE754 == 0)
        {
            func_8009A2A4(D_800FDF58);
        }
    }

    func_80096394();
    index = 0;
    i = 0;
    count = -1;

    do
    {
        if (rec->unk25 != 0xFF)
        {
            if (!(rec->unk1C & 0x1FF) && count <= 0)
            {
                count++;
            }

            if (!(ent->unkC & 0x2000))
            {
                rec->unk40 = func_8006C168(rec);
            }
            else
            {
                rec->unk3C |= 0x1000000;
            }

            mode = ent->unk10 & 0xF;
            if (D_800FE754 == mode || mode == 0)
            {
                func_80086494(i);
                if (!(ent->unk178 & 0x81))
                {
                    if (D_800F229C == 0 && g_field_return_to_title_prompt_state == 0)
                    {
                        func_800880EC(rec);
                    }

                    mode = rec->unk1C & 0x1FF;
                    if (mode == 0)
                    {
                        if (D_80122714 == 0 && D_80122710 == 0 && D_800F229C == 0)
                        {
                            if (!(ent->unkC & 0x21E4))
                            {
                                func_8008DC54(rec, count);
                            }
                            else
                            {
                                func_8006BFC4(rec);
                            }
                        }
                    }
                    else if (mode == 1)
                    {
                        if (D_800F229C == 0 && g_field_return_to_title_prompt_state == 0)
                        {
                            if (!(ent->unkC & 0x21E4))
                            {
                                func_8008D29C(rec, index, 0x600 + (index * 0x400));
                                rec->unk1C &= ~0x800;
                            }
                            else
                            {
                                func_8006BFC4(rec);
                            }
                        }
                        index++;
                    }
                    else if (mode == 2)
                    {
                        if (g_field_return_to_title_prompt_state == 0 && D_800F229C == 0 && D_80122B20 == 0)
                        {
                            if (rec->unk2A != 0x93 && rec->unk2A != 0x94 && rec->unk2A != 0x90 &&
                                rec->unk2A != 0xAE && rec->unk2A != 0x8E && rec->unk2A != 0xB8)
                            {
                                func_80087564(rec);
                            }
                        }

                        if (!(ent->unkC & 0x21E4))
                        {
                            func_8008EF0C(rec);
                            if (g_field_return_to_title_prompt_state == 0 && D_800F229C == 0)
                            {
                                rec->unk1C |= 0x800;
                            }
                        }
                        else
                        {
                            func_8006BFC4(rec);
                        }
                    }

                    func_8008D174(rec);
                }
            }
        }

        i++;
        rec++;
        ent++;
    } while (i < 13);

    func_80091D7C();
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006BFC4(Struct_D800FDF58 *rec)
{
    if (rec->unk4 < 0)
    {
        rec->unk4 += 0x800;
        if (rec->unk4 > 0)
        {
            rec->unk4 = 0;
        }
    }
}

typedef struct
{
    u8 pad0[0x40];
    u32 unk40; /* 0x40 */
    u8 pad44[0x40B8 - 0x44];
    s32 unk40B8; /* 0x40B8 */
} FieldRenderContext;

/**
 * @see decomp.me (100%) TODO
 */
void func_8006BFE8(FieldRenderContext *ctx)
{
    Struct_D800FDF58 *rec;
    Struct_D80105AE0 *ent;
    u32 *base;
    s32 cursor;
    s32 i;

    rec = &D_800FDF58[0];
    base = &ctx->unk40;
    i = 0;
    ent = &D_80105AE0[0];
    cursor = ctx->unk40B8;

    do
    {
        if (rec->unk25 != 0xFE && rec->unk25 != 0xFF)
        {
            if (rec->unk40 >= 0)
            {
                cursor = func_80077FB4(rec, cursor, base, rec->unk40, 0, &D_800FE3A0[i]);
            }
            else
            {
                cursor = func_80075C88(rec, cursor, base, rec->unk40, 0, &D_800FE3A0[i]);
            }
        }
        else if (rec->unk25 == 0xFE)
        {
            ent->unk12C = 0x100000;
            ent->unk140 = -8;
            ent->unk142 = -0xF;
            ent->unk144 = 8;
            ent->unk146 = 0;
        }
        else
        {
            ent->unk12C = 0;
        }

        i++;
        rec++;
        ent++;
    } while (i < 13);

    ctx->unk40B8 = cursor;
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006C11C(u16 arg0)
{
    s32 size;

    size = cdrom_queue_read(arg0);
    cdrom_wait_queue_empty();
    g_field_resource_cursor += (size + 3) & ~3;
}

/**
 * @see decomp.me (100%) TODO
 */
u8 *func_8006C168(Struct_D800FDF58 *rec)
{
    u8 *base;
    u8 *p;
    u8 *q;
    u32 mode;
    s32 wrap;
    s32 shift;
    s32 idx;
    s32 pos;
    s32 at_end;
    u8 seq;
    u8 cursor;
    s32 dur;
    s32 off;
    u32 hi;

    base = g_field_resource_entries[rec->unk3B].start;
    p = base + 4;
    mode = p[1] >> 1;
    wrap = p[1] & 1;
    if (mode & 0x40)
    {
        mode -= 0x40;
    }

    seq = rec->unk21;
    idx = seq & 0x7F;
    if (idx >= (s32)base[4])
    {
        p = base + 6;
        rec->unk21 = seq & 0x80;
    }
    else
    {
        p = p + (idx * 2 + 2);
    }

    off = p[0];
    hi = p[1];
    p = base + off + ((hi & 0x7F) << 8);
    shift = (hi >> 7) + 1;
    rec->unk3C |= 0x1000000;

    if (rec->unk24 != 0)
    {
        rec->unk16--;
        rec->unk34++;
    }

    if (rec->unk16 == 0 && rec->unk24 != 0)
    {
        cursor = rec->unk27 + 1;
        rec->unk27 = cursor;
        if (cursor >= p[0])
        {
            if (rec->unk2E == 0 || --rec->unk2E == 0)
            {
                if (rec->unk1C & 0x800)
                {
                    rec->unk16 = 1;
                    rec->unk34 = 1;
                    rec->unk36 = 0;
                    rec->unk27--;
                    return (u8 *)rec->unk40;
                }
            }
            rec->unk27 = 0;
        }

        pos = rec->unk27;
        rec->unk3C &= 0xFEFFFFFF;
        at_end = (pos + 1) >= (s32)p[0];
        p = p + ((pos << shift) + 1);
        dur = p[1];
        rec->unk16 = dur;
        rec->unk35 = dur;
        if (rec->unk16 == 0)
        {
            rec->unk16++;
            rec->unk35++;
        }
        rec->unk34 = 0;
        if (shift == 2)
        {
            rec->unk37 = p[2];
            rec->unk36 = p[3];
            if (at_end)
            {
                rec->unk38 = rec->unk37;
            }
            else
            {
                rec->unk38 = p[6];
            }
        }
        else
        {
            rec->unk38 = 0;
            rec->unk37 = 0;
            rec->unk36 = mode;
        }
    }
    else
    {
        p = p + ((rec->unk27 << shift) + 1);
    }

    q = base + base[2] + (base[3] << 8) + (p[0] * 2 + 2);
    if (wrap)
    {
        return (u8 *)((s32)(base + q[0] + (q[1] << 8)) & 0x7FFFFFFF);
    }
    return base + q[0] + (q[1] << 8);
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006C3FC(Struct_D800FDF58 *rec)
{
    rec->unk27 = 0;
    rec->unk1C &= ~0x800;
    rec->unk40 = func_8006C460(rec, g_field_resource_entries[rec->unk3B].start);
}

/**
 * @see decomp.me (100%) TODO
 */
u8 *func_8006C460(Struct_D800FDF58 *rec, u8 *base)
{
    u8 *p;
    u8 *q;
    u32 mode;
    s32 wrap;
    s32 shift;
    s32 idx;
    s32 at_end;
    s32 dur;
    s32 off;
    u32 hi;
    u8 seq;

    p = base + 4;
    mode = p[1] >> 1;
    wrap = p[1] & 1;
    if (mode & 0x40)
    {
        mode -= 0x40;
    }

    seq = rec->unk21;
    idx = seq & 0x7F;
    if (idx >= (s32)base[4])
    {
        p = base + 6;
        rec->unk21 = seq & 0x80;
    }
    else
    {
        p = p + (idx * 2 + 2);
    }

    off = p[0];
    hi = p[1];
    p = base + off + ((hi & 0x7F) << 8);
    shift = (hi >> 7) + 1;

    at_end = (rec->unk27 + 1) >= (s32)p[0];
    p = p + 1;
    dur = p[1] * rec->unk24;
    rec->unk16 = dur;
    rec->unk35 = dur;
    if (rec->unk16 == 0)
    {
        rec->unk16++;
        rec->unk35++;
    }
    rec->unk34 = 0;
    if (shift == 2)
    {
        rec->unk37 = p[2];
        rec->unk36 = p[3];
        if (at_end)
        {
            rec->unk38 = 0;
        }
        else
        {
            rec->unk38 = p[6];
        }
    }
    else
    {
        rec->unk38 = 0;
        rec->unk37 = 0;
        rec->unk36 = mode;
    }

    if (rec->unk35 == 0)
    {
        rec->unk35 = 1;
    }
    rec->unk3C &= 0xFEFFFFFF;

    q = base + base[2] + (base[3] << 8) + (p[0] * 2 + 2);
    if (wrap)
    {
        return (u8 *)((s32)(base + q[0] + (q[1] << 8)) & 0x7FFFFFFF);
    }
    return base + q[0] + (q[1] << 8);
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006C5FC(Struct_D800FDF58 *rec)
{
    rec->unk1C |= 0x800;
    rec->unk40 = func_8006C658(rec, g_field_resource_entries[rec->unk3B].start);
}

/**
 * @see decomp.me (100%) TODO
 */
u8 *func_8006C658(Struct_D800FDF58 *rec, u8 *base)
{
    u8 *p;
    u8 *q;
    u32 mode;
    s32 wrap;
    s32 shift;
    s32 idx;
    s32 off;
    u32 hi;
    u8 seq;
    s32 dur;

    p = base + 4;
    mode = p[1] >> 1;
    wrap = p[1] & 1;
    if (mode & 0x40)
    {
        mode -= 0x40;
    }

    seq = rec->unk21;
    idx = seq & 0x7F;
    if (idx >= (s32)base[4])
    {
        p = base + 6;
        rec->unk21 = seq & 0x80;
    }
    else
    {
        p = p + (idx * 2 + 2);
    }

    off = p[0];
    hi = p[1];
    p = base + off + ((hi & 0x7F) << 8);
    shift = (hi >> 7) + 1;

    rec->unk27 = p[0] - 1;
    p = p + ((rec->unk27 << shift) + 1);
    dur = p[1] * rec->unk24;
    rec->unk16 = dur;
    rec->unk35 = dur;
    if (rec->unk16 == 0)
    {
        rec->unk16++;
        rec->unk35++;
    }
    rec->unk34 = 0;
    if (shift == 2)
    {
        rec->unk37 = p[2];
        rec->unk36 = p[3];
        rec->unk38 = 0;
    }
    else
    {
        rec->unk38 = 0;
        rec->unk37 = 0;
        rec->unk36 = mode;
    }

    rec->unk3C &= 0xFEFFFFFF;

    q = base + base[2] + (base[3] << 8) + (p[0] * 2 + 2);
    if (wrap)
    {
        return (u8 *)((s32)(base + q[0] + (q[1] << 8)) & 0x7FFFFFFF);
    }
    return base + q[0] + (q[1] << 8);
}

/**
 * @brief Return the frame count of the animation entry AFTER the record's
 *        current one, or 0 when that entry is past the end of the table.
 * @param rec Field record whose unk3B selects the resource and unk21 the entry.
 * @return Frame count byte of entry (unk21 & 0x7F) + 1, or 0 if out of range.
 * @note WIP - not byte-matching. Insn count and every opcode/offset are exact;
 *       the only defect is one coloring decision. The target keeps `rec` in a2
 *       behind an `addu a2, a0, zero` entry copy because the unk3B index temp
 *       takes a0; ours coalesces the entry copy so `rec` keeps a0 and the temp
 *       goes to a1, which also costs a load-delay nop where the target puts the
 *       base[4] read. local_alloc_oracle --search names the needed edit exactly:
 *       `--drop-sugg 80` (kill the parameter's a0 copy-suggestion) plus
 *       `--add-sugg 80=a2`; dropping the suggestion alone does put the index temp
 *       in a0 but lands `rec` in a1, not a2. No C spelling found reaches it -
 *       see [ENTRY-04] for the four probe classes measured dead here.
 * @see decomp.me (89.19%) TODO
 */
u8 func_8006C7D8(Struct_D800FDF58 *rec)
{
    u8 *base;
    u8 *p;
    s32 idx;

    base = g_field_resource_entries[rec->unk3B].start;
    p = base + 4;
    idx = (rec->unk21 & 0x7F) + 1;
    if (idx >= (s32)base[4])
    {
        return 0;
    }
    p = p + (idx * 2 + 2);
    p = base + p[0] + ((p[1] & 0x7F) << 8);
    return p[0];
}

/**
 * @see decomp.me (100%) TODO
 */
u8 *func_8006C854(Struct_D800FDF58 *rec, u8 *base)
{
    u8 *p;
    u8 *q;
    FieldActorPartDef *part;
    u32 mode;
    s32 wrap;
    s32 shift;
    s32 idx;
    s32 pos;
    s32 at_end;
    s32 dur;
    s32 off;
    u32 hi;
    u8 cursor;

    p = base + 4;
    mode = p[1] >> 1;
    wrap = p[1] & 1;
    if (mode & 0x40)
    {
        mode -= 0x40;
    }

    idx = rec->unk21 & 0x7F;
    if (idx >= (s32)base[4])
    {
        p = base + 6;
    }
    else
    {
        p = p + (idx * 2 + 2);
    }

    off = p[0];
    hi = p[1];
    p = base + off + ((hi & 0x7F) << 8);
    rec->unk3C |= 0x1000000;
    shift = (hi >> 7) + 1;

    if (D_800F2298 == 0 && D_8012269C == 0 && D_801227C8 == 0 && rec->unk24 != 0)
    {
        rec->unk16--;
        rec->unk34++;
    }

    if (rec->unk16 == 0 && rec->unk24 != 0)
    {
        cursor = rec->unk27 + 1;
        rec->unk27 = cursor;
        if (cursor >= p[0])
        {
            part = &g_field_actor_slots[rec->unk22].unk0[rec->unk23];
            if (!((part->unk4 >> 4) & 3))
            {
                func_80071500(rec, part);
                return 0;
            }
            rec->unk27 = 0;
        }

        pos = rec->unk27;
        rec->unk3C &= 0xFEFFFFFF;
        at_end = (pos + 1) == (s32)p[0];
        p = p + ((pos << shift) + 1);
        dur = p[1] * rec->unk24;
        rec->unk34 = 0;
        rec->unk16 = dur;
        rec->unk35 = dur;
        if (shift == 2)
        {
            rec->unk37 = p[2];
            rec->unk36 = p[3];
            if (at_end)
            {
                rec->unk38 = 0;
            }
            else
            {
                rec->unk38 = p[6];
            }
        }
        else
        {
            rec->unk38 = 0;
            rec->unk37 = 0;
            rec->unk36 = mode;
        }
    }
    else
    {
        p = p + ((rec->unk27 << shift) + 1);
    }

    q = base + base[2] + (base[3] << 8) + (p[0] * 2 + 2);
    if (wrap)
    {
        return (u8 *)((s32)(base + q[0] + (q[1] << 8)) & 0x7FFFFFFF);
    }
    return base + q[0] + (q[1] << 8);
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006CAFC(u16 arg0, s32 arg1, s32 arg2)
{
    FieldCdBuffer *buf;
    s32 size;

    buf = D_8010D038;
    size = cdrom_queue_read(arg0, buf);
    cdrom_wait_queue_empty();
    func_8006CB6C(buf, size, arg1, arg2);
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006CB6C(FieldCdBuffer *buf, s32 size, s32 arg2, s32 arg3)
{
    switch (buf->unk0 >> 2)
    {
    case 2:
        func_8006CE00((u8 *)buf + buf->unk4, size - buf->unk4, arg2);
        func_8006CCAC((u8 *)buf + buf->unk0, arg2, 0, arg3);
        break;
    case 3:
        func_8006CE00((u8 *)buf + buf->unk8, size - buf->unk8, arg2);
        func_8006CCAC((u8 *)buf + buf->unk0, arg2, 0, arg3);
        func_8006CCAC((u8 *)buf + buf->unk4, arg2, 1, arg3);
        break;
    case 4:
        func_8006CE00((u8 *)buf + buf->unkC, size - buf->unkC, arg2);
        func_8006CCAC((u8 *)buf + buf->unk0, arg2, 0, arg3);
        func_8006CCAC((u8 *)buf + buf->unk4, arg2, 1, arg3);
        func_8006CCAC((u8 *)buf + buf->unk8, arg2, 2, arg3);
        break;
    }
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006CCAC(FieldCdBuffer *buf, s32 arg1, s32 arg2, s32 arg3)
{
    RECT rect;
    s32 second;

    second = buf->unk8;

    if (arg2 == 2)
    {
        rect.x = 0xC0;
        rect.y = arg3 + 0x1F4;
        rect.w = 0x40;
        rect.h = 1;
    }
    else
    {
        rect.y = arg3 + 0x1F4;
        rect.w = 0x100;
        rect.x = 0;
        rect.h = 1;
    }

    if (arg2 != 1)
    {
        LoadImage(&rect, &buf->unk14);
    }

    if (arg1 >= 2)
    {
        s32 base = 0x340;
        s32 off = arg1 << 6;
        rect.x = base - off;
        rect.w = 0x40;
        rect.y = 0;
        rect.h = 0x100;
    }
    else
    {
        FieldCdBuffer *hdr = (FieldCdBuffer *)(second + (s32)buf);
        s32 w = hdr->unk10;
        s32 h = hdr->unk12;

        if (arg2 == 2)
        {
            rect.x = 0x3C0 - (arg1 << 7);
            rect.y = 0x80;
            rect.w = 0x40;
            rect.h = 0x80;
        }
        else if (arg2 == 1)
        {
            rect.x = 0x3C0 - (arg1 << 7);
            rect.y = 0;
            rect.w = w;
            rect.h = h;
        }
        else
        {
            s32 base = (arg2 << 6) + 0x380;
            s32 off = arg1 << 7;
            rect.x = base - off;
            rect.w = 0x40;
            rect.y = 0;
            rect.h = 0x100;
        }
    }

    LoadImage(&rect, (u8*)(second + (s32)buf + 0x14));
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006CE00(u32 *src, s32 len, s32 slot)
{
    u32 *dst;
    u32 n;

    dst = g_field_resource_cursor;
    n = (u32)(len + 3) >> 2;
    while (n != 0)
    {
        *dst = *src;
        src++;
        n--;
        dst++;
    }

    g_field_resource_entries[slot].start = g_field_resource_cursor;
    g_field_resource_cursor += (len + 3) & ~3;
}

/**
 * @see decomp.me (100%) TODO
 */
s32 func_8006CE70(s32 arg0)
{
    Vec2s pos;
    s32 t;

    pos.x = 0xA0 + D_800F22A0 / 256 + D_800FDF58[arg0].unk0 / 256;
    pos.y = 0x70 + D_800F22A4 / 256 + D_800FDF58[arg0].unk4 / 256 - D_800FDF58[arg0].unk8 / 512 - D_800F22A8 / 512;

    t = pos.x;
    if (t >= 0x10)
    {
        if (t >= 0x131)
        {
            return 0x9F;
        }
        t = ((t - 0x10) * 63) / 288;
        return t + 0x60;
    }
    return 0x60;
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006CF88(void)
{
    u8 *slot1;
    u8 *slot2;
    u8 *pad;
    s32 idx1;
    s32 sel;

    if (D_800FD818[1].unk3 != 0)
    {
        idx1 = D_800FD818[1].unk2 + 2;
    }
    else
    {
        idx1 = (D_800FD818[1].u0.h >> 1) & 1;
    }

    if (D_800FD818[0].unk256 != ((D_800FD818[0].u0.h >> 1) & 1) ||
        D_800FD818[1].unk256 != idx1 ||
        D_800FD818[2].unk256 != D_800FD818[2].unk2 + 0xE)
    {
        if (D_800FD818[1].unk256 != idx1)
        {
            if (D_800FD818[1].unk3 != 0)
            {
                func_800A5174(1, D_800FD818[1].unk2 + 0xA37);
            }
            else
            {
                func_800A5174(1, 0xA37);
            }
        }
        if (D_800FD818[2].unk256 != D_800FD818[2].unk2 + 0xE)
        {
            func_800A5174(2, D_800FD818[2].unk2 + 0xA9B);
        }

        cdrom_stream(0x5E5, D_8010D038);
        cdrom_wait_queue_empty();

        D_800FD818[0].unk256 = (D_800FD818[0].u0.h >> 1) & 1;
        bcopy((u8 *)D_8010D038 + ((u32 *)D_8010D038)[D_800FD818[0].unk256 + 1], g_prim_rect_buf, 0x4A0);

        slot1 = g_prim_rect_buf + 0x4A0;
        D_800FD818[1].unk256 = idx1;
        bcopy((u8 *)D_8010D038 + ((u32 *)D_8010D038)[D_800FD818[1].unk256 + 1], slot1, 0x4A0);

        slot2 = g_prim_rect_buf + 0x940;
        D_800FD818[2].unk256 = D_800FD818[2].unk2 + 0xE;
        bcopy((u8 *)D_8010D038 + ((u32 *)D_8010D038)[D_800FD818[2].unk256 + 1], slot2, 0x4A0);

        if (D_800FD818[1].unk3 == 0)
        {
            func_800A5638(slot1, (D_800FD818[1].u0.h >> 1) & 1);
        }

        if (g_pad_ctx[0xA90] != 0 && (*(u32 *)&g_pad_ctx[0xAA8] & 0x7F) == 4)
        {
            sel = *(s8 *)&g_pad_ctx[0x29D7];
            if (sel < 3)
            {
                pad = g_pad_ctx;
                pad += sel * 0x14C;
                func_800A55E4(slot2, *(s32 *)(pad + 0x2B54));
            }
        }
    }

    func_80084240();
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006D1EC(void)
{
    s32 i;
    s32 val;

    val = 0xFF;
    for (i = 0x102; i >= 0; i--)
    {
        D_800FF658[i].unk25 = val;
    }

    D_80105770 = 0;
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006D21C(FieldActorState *actor)
{
    s32 i;
    s32 owner;
    s32 val;
    Struct_D800FDF58 *base;
    u8 *p;

    owner = actor->unk233;
    i = 0;
    val = 0xFF;
    base = D_800FF658;
    p = (u8 *)base + 0x25;
    while (i < 0x103)
    {
        if (*p != val && p[-3] == owner)
        {
            *p = val;
        }
        i++;
        p += 0x54;
    }
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006D270(FieldActorState *actor)
{
    s32 i;

    if (actor->unk232 != 0)
    {
        for (i = 0; i < actor->unk232; i++)
        {
            if ((actor->unk23A >> i) & 1)
            {
                g_field_track_index = i;
                func_8006D310(actor);
            }
        }
    }
    else
    {
        g_field_track_index = 0;
        func_8006D310(actor);
    }
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006D310(FieldActorState *actor)
{
    FieldActorPartDef *part;
    s32 i;
    s32 val;
    s32 prev;
    s32 count;
    u8 kind;
    u32 flags;

    part = actor->unk0;
    i = 0;
    if (actor->unk25 != 0)
    {
        do
        {
            kind = part->unk31;
            if (kind != 0xFE &&
                (!(actor->unkC->unkC & 0x800) || ((actor->unk240[actor->unk29] >> i) & 1)) &&
                part->unkB != 0xFF &&
                (!(part->unk14 & 4) || g_field_track_index == 0))
            {
                if (actor->unk229[g_field_track_index] == 0xFF)
                {
                    if (kind == 0xFF || (part->unk34 & 0x4000000))
                    {
                        goto next;
                    }
                    if (part->unk31 > actor->unk1EC[g_field_track_index])
                    {
                        goto next;
                    }
                }
                else if (kind != 0xFF)
                {
                    if (part->unk31 > actor->unk1EC[g_field_track_index])
                    {
                        goto next;
                    }
                }

                if (((u8 *)part)[0x2B] & 1)
                {
                    val = part->unkC;
                }
                else
                {
                    val = field_evaluate_parameter_track(actor, part->unkC);
                }

                flags = part->unk28;
                if (((flags >> 30) & 1) && actor->unkCC[g_field_track_index][i] >= val)
                {
                    goto next;
                }

                if ((part->unk0 >> 15) & 1)
                {
                    if (((flags >> 24) & 1) && actor->unk3B[g_field_track_index][i] != 0)
                    {
                        goto next;
                    }
                    if (field_get_track_counter_modulo(actor, (((u8 *)part)[7] & 0xF) + 1) != 0)
                    {
                        goto next;
                    }
                    if (actor->unk3B[g_field_track_index][i] >= val)
                    {
                        goto next;
                    }
                    do
                    {
                        prev = actor->unk3B[g_field_track_index][i];
                        D_80105760 = 0;
                        if (func_8006D79C(actor, i, 0) == -1)
                        {
                            goto next;
                        }
                        if (actor->unk3B[g_field_track_index][i] == prev)
                        {
                            goto next;
                        }
                    } while (actor->unk3B[g_field_track_index][i] < val);
                }
                else
                {
                    if (actor->unk3B[g_field_track_index][i] < val &&
                        field_get_track_counter_modulo(actor, (((u8 *)part)[7] & 0xF) + 1) == 0)
                    {
                        count = (part->unk2C & 0x1F) + 1;
                        while (count != 0)
                        {
                            if (actor->unk3B[g_field_track_index][i] >= val)
                            {
                                break;
                            }
                            D_80105760 = 0;
                            if (func_8006D79C(actor, i, 0) == -1)
                            {
                                break;
                            }
                            count--;
                        }
                    }
                }
            }
        next:
            i++;
            part++;
        } while (i < actor->unk25);
    }
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8006D6A8(Struct_D800FDF58 *dst, FieldActorPartDef *part, Struct_D800FDF58 *rec)
{
    if (!((part->unk4 >> 11) & 1) && !((part->unk28 >> 25) & 1) && (part->unk2C >> 5) == 0 &&
        (*(u32 *)&part->unkC & 0xFFFF0000) == 0x80800000 && part->unk10 == 0x80)
    {
        dst->unk1C |= 0x10008000;
        dst->unk18 = D_800FE3A0[rec->unk3A].unkE;
        dst->unk19 = D_800FE3A0[rec->unk3A].unkF;
        dst->unk1A = D_800FE3A0[rec->unk3A].unk10;
    }
}
