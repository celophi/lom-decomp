#include "common.h"

typedef struct
{
    u8 action_slot_id;
    u8 pad1[0x11];
    u8 record_index;
} FieldMenuCommandState;

typedef struct
{
    u8 pad0[0x26F0];
    s32 handle;
    u8 entry_index;
    u8 entry_state[3];
    u8 counters[8];
} FieldMenuActionSlotView;

extern FieldMenuCommandState D_80122C0D;
extern u8 g_menuLayoutBuffer[];

/**
 * @brief Reset the selected menu action slot to its inactive state.
 * @note Matched under GCC 2.7.2 CDK. Folding the -4 adjustment into
 *       slot_index before constructing the record address makes GCC load
 *       action_slot_id before materializing the global base, matching the
 *       target schedule exactly.
 */
void field_reset_menu_action_slot(void)
{
    FieldMenuActionSlotView *slot;
    u8 *menu;
    s32 slot_index;

    slot_index = D_80122C0D.action_slot_id - 4;
    menu = g_menuLayoutBuffer;
    slot = (FieldMenuActionSlotView *)(menu + (slot_index * 0x10 + D_80122C0D.record_index * 0x8C));
    slot->entry_index = 0xFF;
    slot->handle = 0;
    slot->counters[0] = 0;
    slot->counters[1] = 0;
    slot->counters[2] = 0;
    slot->counters[3] = 0;
    slot->counters[4] = 0;
    slot->counters[5] = 0;
    slot->counters[6] = 0;
    slot->counters[7] = 0;
    *(s32 *)&slot->entry_index |= ~0xFF;
}
