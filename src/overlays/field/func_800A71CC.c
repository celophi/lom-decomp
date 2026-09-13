#include "common.h"

/** @brief Menu display record with packed flags, state, offsets, and update callback. */
typedef struct
{
    union
    {
        u32 word;
        struct
        {
            u16 low;
            s8 priority;
            u8 high;
        } fields;
    } flags;
    union
    {
        u32 word;
        struct
        {
            u16 low;
            s16 height;
        } fields;
    } state;
    s16 scroll_offset;
    s16 scroll_target;
    s16 scroll_tick;
    s16 pad_e;
    void (*update)(void);
} FieldMenuRecord;

void cdrom_queue_read(s32, s32);
void cdrom_wait_queue_empty(void);
void func_800A3938(s32, s32);
void func_800ADF34(void);
FieldMenuRecord *func_800ADF84(void);
extern s32 D_800F229C;
extern s32 D_8010D038;
extern u16 D_80122920;
extern u16 D_80122998;
extern void func_800A8128(void);

/**
 * @brief Load menu data and size its display record for both entry categories.
 */
void func_800A71CC(void)
{
    s16 raw_height;
    s32 height;
    u16 entry_count;
    s32 entry_limit;
    s32 padding;
    s32 index;
    s32 saw_special;
    s32 saw_normal;
    u16 *entry;
    u32 large_state;
    u32 state;
    u32 small_state;
    FieldMenuRecord *record;

    cdrom_queue_read(0x5DF, D_8010D038);
    cdrom_wait_queue_empty();
    func_800A3938(0xA1, 0x80);
    D_800F229C = 3;
    func_800ADF34();
    record = func_800ADF84();
    record->flags.word = (s32)((((record->flags.word & ~0x78) | 8) & 0xFFFF007F) | 0x1000);
    saw_normal = 0;
    saw_special = 0;
    index = 0;
    padding = 0;
    entry_count = D_80122998;
    if (entry_count != 0)
    {
        entry_limit = entry_count;
        entry = &D_80122920;
        do
        {
            if (*entry & 0x8000)
            {
                if (saw_special == 0)
                {
                    saw_special = 1;
                    padding += 0x10;
                }
            }
            else if (saw_normal == 0)
            {
                saw_normal = 1;
                padding += 0x10;
            }
            index += 1;
            entry++;
        } while (index < entry_limit);
    }

    record->update = &func_800A8128;
    record->flags.word = (s32)(record->flags.word & 0xFFFFFF);
    state = record->state.word;
    state |= 1;
    record->state.word = state;
    raw_height = (D_80122998 * 0x10) + padding;
    record->state.word = state | 0x200;
    record->state.fields.height = raw_height;
    height = raw_height;
    if (height < 0xA1)
    {
        small_state = (record->state.word & ~0x1FE) | ((height & 0xFF) * 2);
        record->state.word = small_state;
        record->flags.fields.priority = (s8)(0x70 - ((small_state >> 2) & 0x7F));
        return;
    }

    record->scroll_offset = 0;
    record->scroll_target = 0;
    record->scroll_tick = 0;
    large_state = (((record->state.word & ~0xC00) | 0x400) & ~0x1FE) | 0x140;
    record->state.word = large_state;
    record->flags.fields.priority = (s8)(0x70 - ((large_state >> 2) & 0x50));
}
