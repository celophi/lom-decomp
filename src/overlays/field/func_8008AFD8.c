#include "common.h"

/** @brief Actor position, direction, and animation fields in the 0x54-byte entry. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 pad0C[0x21 - 0xC];
    u8 unk21;
    u8 pad22[2];
    s8 unk24;
    u8 pad25[2];
    s8 unk27;
    u8 pad28[6];
    s16 unk2E;
    u8 pad30[10];
    u8 unk3A;
    u8 unk3B;
    u8 pad3C[0x54 - 0x3C];
} Entry;
/** @brief Actor identity and movement flags in the 0x23C-byte record. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x174 - 0x18];
    s32 unk174;
    u8 pad178[0x23C - 0x178];
} Actor;
/** @brief Resource flags selecting the direction mode. */
typedef struct
{
    u8 pad0[0x10];
    s32 unk10;
} Resource;
extern Entry D_800FDF58[];
extern Actor D_80105AE0[];
extern Resource g_field_resource_entries[];
extern s32 ratan2(s32 y, s32 x);
extern void func_8006C3FC(Entry *entry);
/**
 * @brief Turn the source actor toward the target and reset its movement state.
 * @param source_id Source actor identifier.
 * @param target_id Target actor identifier.
 * @return Zero on success, or -1 if either actor is absent.
 */
s32 func_8008AFD8(s32 source_id, s32 target_id)
{
    Entry *first;
    Entry *second;
    Entry *entry;
    Actor *actor;
    Entry *entry2;
    Actor *actor2;
    Actor *base;
    Actor *slot;
    s32 i;
    s32 angle;
    u8 direction;
    entry = D_800FDF58;
    actor = D_80105AE0;
    for (i = 0; i < 13; i++, actor++, entry++)
    {
        if (actor->unk14 == source_id)
        {
            goto first_found;
        }
    }
    first = (Entry *)-1;
first_check:
    if (first != (Entry *)-1)
    {
        goto second_start;
    }
    return -1;
first_found:
    first = entry;
    goto first_check;
second_found:
    second = entry2;
    goto second_check;
second_start:
    entry2 = D_800FDF58;
    actor2 = D_80105AE0;
    for (i = 0; i < 13; i++, actor2++, entry2++)
    {
        if (actor2->unk14 == target_id)
        {
            goto second_found;
        }
    }
    second = (Entry *)-1;
second_check:
    if (second == (Entry *)-1)
    {
        goto fail;
    }
    angle = ratan2(first->unk8 - second->unk8, second->unk0 - first->unk0);
    if (!(g_field_resource_entries[first->unk3B].unk10 & 1))
    {
        if (angle < -0x700)
        {
            direction = 2;
            goto store_direction;
        }
        else if (angle < -0x500)
        {
            direction = 3;
            goto store_direction;
        }
        else if (angle < -0x300)
        {
            direction = 4;
            goto store_direction;
        }
        else if (angle < -0x100)
        {
            direction = 0x83;
            goto store_direction;
        }
        else if (angle < 0x100)
        {
            direction = 0x82;
            goto store_direction;
        }
        else if (angle < 0x300)
        {
            direction = 0x81;
            goto store_direction;
        }
        else if (angle < 0x500)
        {
            goto zero_direction;
        }
        else if (angle < 0x700)
        {
            direction = 1;
            goto store_direction;
        }
        else
        {
            direction = 2;
            goto store_direction;
        }
    }
    else
    {
        if ((angle > 0x400 && angle < 0xC00) || (angle < -0x400 && angle > -0xC00))
        {
            goto zero_direction;
        }
        else
        {
            direction = 0x80;
            goto store_direction;
        }
    }
zero_direction:
    first->unk21 = 0;
    goto reset_state;
store_direction:
    first->unk21 = direction;
reset_state:
    base = D_80105AE0;
    first->unk2E = 1;
    first->unk27 = 0;
    first->unk24 = 1;
    slot = &base[first->unk3A];
    slot->unk174 &= ~0x1800;
    func_8006C3FC(first);
    return 0;
fail:
    return -1;
}
