#include "common.h"
#include "game_audio.h"

extern u32 *D_80123FB8;

s32 func_800BA228(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)D_80123FB8, 0x20);
    return *D_80123FB8 &= 0x7FFFFFFF;
}

s32 func_800BA278(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)D_80123FB8, 0x21);
    return *D_80123FB8 &= 0x7FFFFFFF;
}

s32 func_800BA2C8(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)D_80123FB8, 0x23);
    return *D_80123FB8 &= 0x7FFFFFFF;
}

s32 func_800BA318(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)D_80123FB8, 0x23);
    return *D_80123FB8 &= 0x7FFFFFFF;
}
