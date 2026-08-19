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
    u32 unk0; /* 0x00 */
    u32 unk4; /* 0x04 */
    u32 unk8; /* 0x08 */
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
    s8 unk22; /* 0x22 */
    u8 pad23[0x24 - 0x23];
    u8 unk24; /* 0x24 */
    u8 unk25; /* 0x25 */
    u8 pad26[0x27 - 0x26];
    u8 unk27; /* 0x27 */
    u8 unk28; /* 0x28 */
    u8 pad29[0x2A - 0x29];
    s16 unk2A; /* 0x2A */
    s16 unk2C; /* 0x2C */
    s16 unk2E; /* 0x2E */
    s16 unk30; /* 0x30 */
    u8 unk32; /* 0x32 */
    u8 unk33; /* 0x33 */
    u8 unk34; /* 0x34 */
    u8 pad35[0x3A - 0x35];
    u8 unk3A; /* 0x3A */
    u8 unk3B; /* 0x3B */
    u8 pad3C[0x40 - 0x3C];
    u32 unk40; /* 0x40 */
    u32 unk44; /* 0x44 */
    u32 unk48; /* 0x48 */
    u32 unk4C; /* 0x4C */
    u8 pad50[0x54 - 0x50];
} Struct_D800FDF58;

typedef struct
{
    u8 pad0[0x178];
    s32 unk178; /* 0x178 */
    u8 pad17C[0x18E - 0x17C];
    u8 unk18E; /* 0x18E */
    u8 pad18F[0x19C - 0x18F];
    s32 unk19C; /* 0x19C */
    s32 unk1A0; /* 0x1A0 */
    u8 pad1A4[0x23C - 0x1A4];
} Struct_D80105AE0;

extern Struct_D800FDF58 D_800FDF58[];
extern Struct_D80105AE0 D_80105AE0[];
extern void* g_field_resource_cursor;
extern u8 D_800FDA83;
extern u8 D_800FDCEA;
extern u8 *g_pad_ctx;

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
    u8 padA[0xD - 0xA];
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
    u8 pad2C[0x2E - 0x2C];
    u8 unk2E; /* 0x2E */
    u8 pad2F[0x33 - 0x2F];
    u8 unk33; /* 0x33 */
    u8 pad34[0x48 - 0x34];
} FieldActorPartDef;

extern FieldActorPartDef D_800FE3A0[];

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
