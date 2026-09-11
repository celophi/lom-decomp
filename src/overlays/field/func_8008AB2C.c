#include "common.h"

/** @brief Field entry with animation state and actor index. */
typedef struct
{
    u8 pad0[0x2A];
    s16 unk2A;
    u8 pad2C[14];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} Entry;
/** @brief Actor record with lookup ID, state flags, and action byte. */
typedef struct
{
    s32 unk0, unk4, unk8, unkC, unk10, unk14;
    u8 pad18[0x16F - 0x18];
    u8 unk16F;
    u8 pad170[8];
    s32 unk178;
    u8 pad17C[0x23C - 0x17C];
} Actor;
/** @brief Actor data record containing the pending action byte. */
typedef struct
{
    u8 pad0[0x259];
    u8 unk259;
    u8 pad25A[14];
} Data;
/** @brief Input context view containing the actor-zero event counter. */
typedef struct
{
    u8 pad0[0x3154];
    s32 unk3154;
} Pad;
extern Entry D_800FDF58[];
extern Actor D_80105AE0[];
extern Data D_800FD818[];
extern Pad *g_pad_ctx;
extern s32 D_8010A000;
extern void func_8008B870(Entry *, s32);
/**
 * @brief Mark an actor for an action and dispatch its state-dependent update.
 * @param arg0 Actor ID to locate among the field records.
 * @param arg1 Reaction-mode selector forwarded to func_8008B870.
 * @return Zero when found, or -1 when no record has the requested ID.
 */
s32 func_8008AB2C(s32 arg0, s32 arg1)
{
    Entry *entry_cursor;
    Actor *actor_cursor;
    Entry *entry;
    s16 action;
    s32 pad_counter;
    s32 entry_count;
    Actor *actor;
    u8 *actor_base;

    entry_cursor = D_800FDF58;
    actor_cursor = D_80105AE0;
    entry_count = 0;
loop_1:
    entry_count += 1;
    if (actor_cursor->unk14 != arg0)
    {
        actor_cursor++;
        entry_cursor++;
        if (entry_count >= 0xD)
        {
            entry = (Entry *)-1;
        }
        else
        {
            goto loop_1;
        }
    }
    else
    {
        goto found;
    }
check:
    if (entry != (Entry *)-1)
    {
        goto body;
    }
    return -1;
found:
    entry = entry_cursor;
    goto check;
body:
    if (entry->unk3A == 0)
    {
        pad_counter = g_pad_ctx->unk3154;
        if (pad_counter != -1)
        {
            g_pad_ctx->unk3154 = (s32)(pad_counter + 1);
        }
    }
    actor_base = (u8 *)D_80105AE0;
    actor = (Actor *)(actor_base + entry->unk3A * 0x23C);
    actor->unkC = (s32)(actor->unkC | 0x10000000);
    if ((u8)entry->unk3A < 3U)
    {
        D_800FD818[entry->unk3A].unk259 = 5;
    }
    else if (((Actor *)(actor_base + entry->unk3A * 0x23C))->unk8 < 0)
    {
        D_8010A000 = 5;
    }
    action = entry->unk2A;
    if (action == 0x87)
    {
        return 0;
    }
    if (action >= 0x88)
    {
        if (action != 0x91)
        {
            func_8008B870(entry, arg1);
        }
        return 0;
    }
    if (action >= 0x85)
    {
        switch (((Actor *)((u8 *)D_80105AE0 + entry->unk3A * 0x23C))->unk16F)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 8:
        case 9:
        case 10:
            break;
        default:
            if (!(((u32)((Actor *)((u8 *)D_80105AE0 + entry->unk3A * 0x23C))->unk178 >> 6) & 1))
            {
                return 0;
            }
            break;
        }
    }
    func_8008B870(entry, arg1);
    return 0;
}
