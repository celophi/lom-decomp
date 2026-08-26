#include "common.h"
#include "game_audio.h"

extern u32 *D_80123FB8;

s32 func_800BA3D0(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)D_80123FB8, 0x25);
    return *D_80123FB8 &= 0x7FFFFFFF;
}

s32 func_800BA420(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)D_80123FB8, 0x26);
    return *D_80123FB8 &= 0x7FFFFFFF;
}
