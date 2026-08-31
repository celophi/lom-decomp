#include "common.h"

typedef struct
{
    s16 unk0;
    s16 unk2;
} FieldC7494State;

extern FieldC7494State D_80122C10;
extern u8 g_menuLayoutBuffer[];
extern void akao_set_song_params(s32 flags, s32 duration, s32 field_id, s32 sub_id);

void func_800C7494(void)
{
    s32 idx;
    s32 val;
    s32 count;
    s32 row;
    u8 *base;
    u8 *slot;

    idx = D_80122C10.unk0;
    val = D_80122C10.unk2;
    if (idx < 5)
    {
        count = 0;
        base = g_menuLayoutBuffer;
        row = idx * 0x60;
        do
        {
            slot = (u8 *)((count + row) + (s32)base);
            count++;
            if ((u32)((slot[0x2F38] + 2) & 0xFF) < 2)
            {
                slot[0x2F38] = val;
                return;
            }
        } while (count < 3);
        return;
    }
    akao_set_song_params(0x8002, 0x30, idx, 0);
}
