#include "common.h"

typedef struct
{
    u8 pad[0x48];
    u16 unk48;
} StructB31CCSub;

typedef struct
{
    u8 pad[0x10];
    StructB31CCSub *unk10;
} StructB31CC;

extern u8 *D_80122B74;

void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);
StructB31CC *func_800B2A9C(s32 value);
void func_800BD520(s32 arg0, s32 arg1, s32 arg2);
u32 func_800C9ED4(s32 arg0);

/**
 * @see decomp.me (100%)
 */
void func_800B31CC(s32 arg0)
{
    u32 chance;
    u8 pad[0x20]; /* unreferenced; reserves the original's stack slot */
    u32 count;
    u32 index;

    chance = D_80122B74[0xC06];
    if ((u32)(rand() % 100) < chance)
    {
        func_800BD520(2, 0xD028, 0x64);
    }
    else
    {
        count = D_80122B74[0xC04] >> 4;
        if ((count < 4) || (count >= 8))
        {
            akao_set_song_params(0x74, count, 0, 0);
            func_800BD520(2, 0xD028, 0x63);
        }
        index = func_800C9ED4(arg0);
        if (index >= count)
        {
            index = count - 1;
        }
        func_800BD520(2, 0xD028, (((func_800B2A9C(2)->unk10->unk48 * count) >> 8) * 6) + index);
    }
}
