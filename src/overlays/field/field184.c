#include "common.h"

extern s32 g_gosub_result_values[];
extern u8 g_menuLayoutBuffer[];

/**
 * @brief Flag the current gosub result's menu layout entry, or trigger a song-select cue.
 * @note Calls akao_set_song_params with no prototype in scope, matching the field116.c
 *       convention; this is required to match.
 */
void func_800C71D4(void)
{
    s32 idx;
    u8 *base;
    u8 *rec;

    idx = g_gosub_result_values[0];
    if (idx < 5)
    {
        base = g_menuLayoutBuffer;
        rec = &base[idx * 0x60];
        *(u32 *)(rec + 0x2F38) |= 0x40000000;
    }
    else
    {
        akao_set_song_params(0x8002, 0x29, idx, 0);
    }
}

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} UnkStruct80051EC0;

extern UnkStruct80051EC0 D_80051EC0;
extern void func_800AAFEC(UnkStruct80051EC0 *arg0);

/**
 * @brief Copy the D_80051EC0 constant onto the stack and forward it to func_800AAFEC.
 */
void func_800C7238(void)
{
    UnkStruct80051EC0 local;

    local = D_80051EC0;
    func_800AAFEC(&local);
}
