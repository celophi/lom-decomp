#include "common.h"

/**
 * @brief Active field entry fields used by the state update.
 */
typedef struct EntryB8964
{
    u8 pad0[8];
    s32 value;
    u32 flags;
} EntryB8964;

/**
 * @brief Field state header containing the active entry index.
 */
typedef struct StateB8964
{
    u8 pad0[4];
    s32 index;
} StateB8964;

extern StateB8964 *D_80123FB8;

/**
 * @brief Rewrites the active entry's flag bit zero and bumps its counter.
 *
 * The active entry uses a 12-byte stride. Flag bit zero is replaced with the
 * value of flag bit one, then the entry value at offset eight is incremented.
 */
void func_800B8964(void)
{
    EntryB8964 *entry;
    EntryB8964 *entry2;
    u32 flags;
    u32 masked;

    entry = (EntryB8964 *)((u8 *)D_80123FB8 + D_80123FB8->index * 0xC);
    flags = entry->flags;
    masked = flags & ~1;
    masked |= (flags >> 1) & 1;
    entry->flags = masked;
    entry2 = (EntryB8964 *)((u8 *)D_80123FB8 + D_80123FB8->index * 0xC);
    entry2->value++;
}
