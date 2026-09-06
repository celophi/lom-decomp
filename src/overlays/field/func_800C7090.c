#include "common.h"

typedef struct
{
    s16 unk0;
    s16 unk2;
} FieldC7090State;

extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values;
extern u8 g_menuLayoutBuffer[];
extern FieldC7090State D_80122C10;
extern u16 D_80122C16;
extern s32 D_80045EC8;
extern void akao_set_song_params(s32 flags, s32 duration, s32 field_id, s32 sub_id);

/**
 * @see decomp.me (100%)
 */
void func_800C7090(void)
{
    s32 idx;
    u8 *base;
    u8 *rec;
    s32 flags;

    if (g_gosub_result_count != 0)
    {
        idx = g_gosub_result_values;
        if (idx < 5)
        {
            base = g_menuLayoutBuffer;
            rec = base + idx * 0x60;
            flags = *(s32 *)(rec + 0x2F38);
            if (flags < 0)
            {
                D_80122C10.unk0 = rec[0x2F0A] + 0x53;
                D_80122C10.unk2 = 0;
            }
            else
            {
                s16 masked = (s16)(((u32) flags >> 30) & 1);
                D_80122C10.unk0 = rec[0x2F09] + 0x12;
                D_80122C10.unk2 = masked;
            }
            if (idx == D_80045EC8)
            {
                D_80122C10.unk0 = 0xFE;
            }
        }
        else
        {
            akao_set_song_params(0x8002, 0x27, idx, 0);
        }
    }
    D_80122C16 = (u16) g_gosub_result_count;
}
