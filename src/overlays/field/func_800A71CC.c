#include "common.h"
/** @brief Menu record with packed flags, height, and update callback. */
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
    s16 scroll_offset, scroll_target, scroll_tick, pad_e;
    void (*update)(void);
} Record;
void cdrom_queue_read(s32, s32);
void cdrom_wait_queue_empty(void);
void func_800A3938(s32, s32);
void func_800ADF34(void);
Record *func_800ADF84(void);
extern s32 D_800F229C;
extern s32 D_8010D038;
extern u16 D_80122920;
extern u16 D_80122998;
extern void func_800A8128(void);

/**
 * @brief Load menu data and size its display record for both entry categories.
 * @note Adds one 16-pixel header for each category present in the entry list.
 * @note Best current match: 86.227270% with GCC 2.7.2 CDK.
 */
void func_800A71CC(void)
{
    s16 height;
    s32 count;
    s32 padding;
    s32 index;
    s32 saw_special;
    s32 saw_normal;
    u16 *entry;
    u32 large_state;
    u32 state;
    u32 small_state;
    Record *record;

    cdrom_queue_read(0x5DF, D_8010D038);
    cdrom_wait_queue_empty();
    func_800A3938(0xA1, 0x80);
    D_800F229C = 3;
    func_800ADF34();
    record = func_800ADF84();
    saw_normal = 0;
    saw_special = 0;
    index = 0;
    padding = 0;
    record->flags.word = (s32)((((record->flags.word & ~0x78) | 8) & 0xFFFF007F) | 0x1000);
    count = D_80122998;
    if (count != 0)
    {
        entry = &D_80122920;
        do
        {
            if (*entry & 0x8000)
            {
                if (saw_special == 0)
                {
                    saw_special = 1;
                    goto add_padding;
                }
            }
            else if (saw_normal == 0)
            {
                saw_normal = 1;
            add_padding:
                padding += 0x10;
            }
            index += 1;
            entry++;
        } while (index < count);
    }
    record->update = &func_800A8128;
    record->flags.word = (s32)(record->flags.word & 0xFFFFFF);
    state = record->state.word | 1;
    do
    {
        record->state.word = state;
    } while (0);
    record->state.word = (u32)(state | 0x200);
    height = (D_80122998 * 0x10) + padding;
    record->state.fields.height = height;
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
