#include "common.h"

/**
 * @brief Active field entry fields used by the state update.
 */
typedef struct EntryB8858
{
    u8 pad0[8];
    s32 value;
    u32 flags;
} EntryB8858;

/**
 * @brief Field state header containing the active entry index.
 */
typedef struct StateB8858
{
    u8 pad0[4];
    s32 index;
} StateB8858;

extern StateB8858 *g_field_script;
void func_800BD128(s32);

/**
 * @brief Advances the active field entry or dispatches its flagged action.
 *
 * The active entry uses a 12-byte stride. Flag bit zero dispatches action one;
 * otherwise the entry value at offset eight advances by three.
 */
void func_800B8858(void)
{
    EntryB8858 *entry;

    entry = (EntryB8858 *)((u8 *)g_field_script + g_field_script->index * 0xC);
    if (entry->flags & 1)
    {
        func_800BD128(1);
        return;
    }
    entry->value += 3;
}
