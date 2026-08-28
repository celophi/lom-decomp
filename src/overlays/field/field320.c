#include "common.h"

typedef struct {
    u8 pad0[0xA];
    u16 unkA;   /* 0x0A */
    u8 padC[0x14 - 0xC];
} ResEntry;

typedef struct {
    u8 pad0[3];
    u8 unk3;    /* 0x03 */
    u8 pad4[0x268 - 4];
} SrcEntry;

extern ResEntry g_field_resource_entries[];
extern SrcEntry D_800FD818[];
extern u16 D_800EB2B4[];
extern s32 D_801178B4;

/**
 * @see decomp.me (100%) TODO
 */
void func_8009C434(void)
{
    s32 i;

    if (D_801178B4 >= 6)
    {
        D_801178B4 = 5;
    }
    i = 0;
    do
    {
        if (D_800FD818[i].unk3 == 0)
        {
            g_field_resource_entries[i].unkA = D_800EB2B4[D_801178B4];
        }
        else
        {
            g_field_resource_entries[i].unkA = 0;
        }
        i += 1;
    } while (i < 2);
}
