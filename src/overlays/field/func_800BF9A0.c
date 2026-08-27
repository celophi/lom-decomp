#include "common.h"

typedef struct FieldEntry
{
    u8 pad0[0xC];
    u8 value;
    u8 count;
} FieldEntry;

typedef struct FieldState800BF9A0
{
    u8 pad0[8];
    s32 value;
} FieldState800BF9A0;

extern void *D_80123FC4;

/**
 * @brief Decrements an entry counter and adds its weighted value to the field
 *        state.
 *
 * @param index Index of the two-byte field entry.
 */
void func_800BF9A0(s32 index)
{
    s32 offset;
    u8 count;
    FieldEntry *entry;
    FieldEntry *entry2;

    offset = index * 2;
    entry = D_80123FC4 + offset;
    count = entry->count;
    if (count != 0)
    {
        entry->count = count - 1;
        entry2 = D_80123FC4 + offset;
        ((FieldState800BF9A0 *)D_80123FC4)->value += entry2->value << entry2->count;
    }
}
