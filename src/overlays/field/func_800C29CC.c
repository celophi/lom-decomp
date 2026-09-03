#include "common.h"

void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);
u8 *func_800C1E40(s32 arg0);
void *func_800A9060(void);
s32 func_800B2844(s32 arg0, u8 *arg1, s32 arg2);
void func_800A8F8C(void *arg0, u8 *arg1);

/**
 * @see decomp.me (100%)
 */
s32 func_800C29CC(s32 index)
{
    u8 *table;
    u8 *entry;
    void *object;

    table = func_800C1E40(5);
    if (table == NULL)
    {
        akao_set_song_params(0x8001, 0x6C, index, 0);
        return -1;
    }
    if (index >= *(u16 *)(table + 2))
    {
        akao_set_song_params(0x8001, 0x6C, index, 1);
        return -1;
    }

    object = func_800A9060();
    entry = table + ((index * 0x40) + 4);
    func_800B2844(0, entry, 0x15);
    if (object != NULL)
    {
        func_800A8F8C(object, entry);
        return 0;
    }
    return -1;
}
