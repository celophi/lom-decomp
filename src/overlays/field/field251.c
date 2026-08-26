#include "common.h"
#include "game_audio.h"
extern u32 *D_80123FB8;
s32 func_800BB584(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)D_80123FB8, 0x39);
    return *D_80123FB8 &= 0x7FFFFFFF;
}

s32 func_800BB5D4(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)D_80123FB8, 0x3A);
    return *D_80123FB8 &= 0x7FFFFFFF;
}

s32 func_800BB624(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)D_80123FB8, 0x3B);
    return *D_80123FB8 &= 0x7FFFFFFF;
}

s32 func_800BB674(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)D_80123FB8, 0x3C);
    return *D_80123FB8 &= 0x7FFFFFFF;
}

s32 func_800BB6C4(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)D_80123FB8, 0x3D);
    return *D_80123FB8 &= 0x7FFFFFFF;
}

s32 func_800BB714(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)D_80123FB8, 0x3E);
    return *D_80123FB8 &= 0x7FFFFFFF;
}

s32 func_800BB764(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)D_80123FB8, 0x3F);
    return *D_80123FB8 &= 0x7FFFFFFF;
}
