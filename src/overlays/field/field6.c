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
    u8 pad0[0x25];
    u8 unk25; /* 0x25 */
    u8 pad26[0x3B - 0x26];
    u8 unk3B; /* 0x3B */
    u8 pad3C[0x40 - 0x3C];
    u32 unk40; /* 0x40 */
    u8 pad44[0x54 - 0x44];
} Struct_D800FDF58;

extern Struct_D800FDF58 D_800FDF58[];
extern void* g_field_resource_cursor;

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
 * @see decomp.me (97.84%) TODO
 * @note Residual: same compiler-temp coloring class documented as unresolved
 *       in the sibling func_8006AB38 (src/overlays/field/field4.c, 97.74%,
 *       see working/func_8006AB38/STATUS.md) - a 2-insn gap around the
 *       scaled-index temp right after the entry guard branch. Not source-
 *       shape controllable via local probing; a permuter run found nothing.
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
