#include "common.h"

#define FIELD_MENU_SLOT_UNUSED 0xFF
#define FIELD_MENU_SLOT_STRIDE 0x10
#define FIELD_MENU_RECORD_STRIDE 0x8C
#define FIELD_ACTION_ENTRY_FLAG_MASK 0xF

typedef struct
{
    u8 pad0[0x26F0];
    s32 handle;
    u8 entry_index;
    u8 entry_state[3];
} FieldMenuActionSlot;

typedef struct
{
    s8 id;
    u8 pad1[0x8F];
    s32 flags;
} FieldActionEntry;

typedef struct
{
    u8 pad0[0x400];
    u16 count;
    u8 pad1[0x2E];
    FieldActionEntry entries[1];
} FieldActionTable;

typedef struct
{
    s32 flags;
    u8 pad4[8];
    s16 result_type;
} FieldActionRequest;

extern u8 *D_80122B74;
extern FieldActionTable *D_80122B78;

/**
 * @brief Resolve a menu action slot and append its table entry when active.
 * @param arg0 Action request containing packed slot selection and result state.
 * @param arg1 Receives the selected table entry, or NULL when no entry is active.
 * @param arg2 Value used to initialize the selected entry identifier.
 * @param arg3 Receives the selected action index when required by the action type.
 * @return -1 when an entry is appended, or 0 when no entry is selected.
 */
s32 func_800B1894(FieldActionRequest *arg0, FieldActionEntry **arg1, s32 arg2, s32 *arg3)
{
    FieldMenuActionSlot *slot;
    FieldActionEntry *entry;
    FieldActionRequest *request;
    s32 offset;
    u32 handle;
    s32 count;
    s32 index;
    s16 result_type;
    u32 record_index;
    u8 packed;

    request = arg0;
    packed = ((u8 *)request)[1];
    record_index = packed >> 7;
    packed &= 7;
    offset = packed * FIELD_MENU_SLOT_STRIDE + record_index * FIELD_MENU_RECORD_STRIDE;
    slot = (FieldMenuActionSlot *)(D_80122B74 + offset);

    if (slot->entry_index < FIELD_MENU_SLOT_UNUSED)
    {
        handle = slot->handle;
        switch (handle)
        {
        case 0:
            *arg1 = NULL;
            return 0;

        case 1:
            *arg3 = 0;
            request->result_type = 2;
            break;

        case 2:
            *arg3 = slot->entry_index - 0x30;
            result_type = (s16)((*(u32 *)&((FieldMenuActionSlot *)(D_80122B74 + offset))->entry_index >> 8) & 3);
            request->result_type = result_type;
            break;

        case 3:
            *arg3 = slot->entry_index;
            result_type = (s16)((*(u32 *)&((FieldMenuActionSlot *)(D_80122B74 + offset))->entry_index >> 8) & 3);
            request->result_type = result_type;
            break;

        default:
            break;
        }

        count = D_80122B78->count;
        D_80122B78->count = (u16)(count + 1);
        index = count & 0xFFFF;
        entry = &D_80122B78->entries[index];
        *arg1 = entry;
        entry->flags |= 0x80000000;
        (*arg1)->id = (s8)(arg2 + 3);
        (*arg1)->flags = ((*arg1)->flags & ~FIELD_ACTION_ENTRY_FLAG_MASK) | (request->flags & FIELD_ACTION_ENTRY_FLAG_MASK);
        request->flags &= ~FIELD_ACTION_ENTRY_FLAG_MASK;
        return -1;
    }

    *arg1 = NULL;
    return 0;
}
