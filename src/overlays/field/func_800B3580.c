#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    u8 *unk4;
    u8 *unk8;
    u8 unkC[8];
    s32 unk14;
    s32 unk18;
} StructB3580;

extern u8 D_800EF8C0[];
extern u8 D_800F0B48[];
typedef struct
{
    u8 pad[0x2F4];
    u8 unk2F4[1][0xC];
} StructB74;

extern StructB74 *D_80122B74;
extern StructB3580 D_80123B08;
extern u8 *D_80123FAC;
extern StructB3580 *D_80123FB0;
extern u16 g_music_track_index;

void func_800C1EC8(s32 arg0, void *arg1, s32 arg2);
u8 *func_800C1E40(s32 arg0);
void func_800BD520(s32 arg0, s32 arg1, s32 arg2);
s32 func_800B3670(s32 arg0);

/**
 * @see decomp.me (100%)
 */
void func_800B3580(void)
{
    s32 i;
    u8 *p;

    D_80123FAC = D_800EF8C0;
    D_80123FB0 = &D_80123B08;
    func_800C1EC8(0, &D_80123B08, 0x4A4);
    D_80123FB0->unk18 = 0;
    D_80123FB0->unk0 = func_800B3670(0);

    for (i = 0; i < 8; i++)
    {
        D_80123FB0->unkC[i] = D_800F0B48[D_80122B74->unk2F4[g_music_track_index][i]];
    }

    p = func_800C1E40(1);
    D_80123FB0->unk4 = p + *(s32 *)(p + 4);
    D_80123FB0->unk8 = p + *(s32 *)(p + 8);
    func_800BD520(0, 0x428C, -1);
}
