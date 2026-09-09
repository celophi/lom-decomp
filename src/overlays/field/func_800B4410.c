#include "common.h"

extern u8 *D_80122B78;
extern u8 *D_80122B74;

void func_80087614(s32 arg0, s32 arg1);
void func_800B28E0(s32 arg0, s32 arg1, s32 arg2);
void func_800966F0(s32 arg0, void *arg1);
s32 func_80087FC0(s32 arg0, s32 arg1);

/**
 * @brief Notify actors matching a status key and update the three idle slots.
 * @param arg0 Status key to broadcast and store in the shared field state.
 * @note Preserve the loop-variable record-count comparison and repeated pointer reads.
 * @note WIP: pointer reload and register-allocation differences remain.
 */
void func_800B4410(s32 arg0)
{
    s32 off;
    s32 i;
    s32 off2;

    i = 3;
    if (i < *(u16 *)(D_80122B78 + 0x400))
    {
        off = 0x1BC;
        do
        {
            if ((*(u32 *)(D_80122B78 + off + 0x4C0) & 0xF) == arg0)
            {
                func_80087614(*(u8 *)(D_80122B78 + off + 0x430), arg0);
                func_800B28E0(*(u8 *)(D_80122B78 + off + 0x430), 0xD, 0);
            }
            i++;
            off += 0x94;
        } while (i < *(u16 *)(D_80122B78 + 0x400));
    }

    *(u32 *)(D_80122B78 + 0x400) |= 0x10000;
    *(u8 *)(D_80122B78 + 0x403) = arg0;
    func_800966F0(arg0, D_80122B78);

    off = 0;
    off2 = 0;
    for (i = 0; i < 3; i++)
    {
        if ((*(u8 *)(D_80122B74 + off2 + 0x608) >> 7) != 0)
        {
            func_80087FC0(i, 0);
        }
        else
        {
            *(u16 *)(D_80122B78 + off + 0x436) = 0xFFFF;
            func_80087FC0(i, 2);
            func_800B28E0(*(u8 *)(D_80122B78 + off + 0x430), 0xF, 0);
        }
        off += 0x94;
        off2 += 0x250;
    }

    func_800B28E0(0x80, 0xD, 0);
}
