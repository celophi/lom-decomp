#include "common.h"
#include "game_audio.h"
extern u32 *g_field_script;
s32 func_800BB584(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)g_field_script, 0x39);
    return *g_field_script &= 0x7FFFFFFF;
}

s32 func_800BB5D4(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)g_field_script, 0x3A);
    return *g_field_script &= 0x7FFFFFFF;
}

s32 func_800BB624(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)g_field_script, 0x3B);
    return *g_field_script &= 0x7FFFFFFF;
}

s32 func_800BB674(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)g_field_script, 0x3C);
    return *g_field_script &= 0x7FFFFFFF;
}

s32 func_800BB6C4(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)g_field_script, 0x3D);
    return *g_field_script &= 0x7FFFFFFF;
}

s32 func_800BB714(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)g_field_script, 0x3E);
    return *g_field_script &= 0x7FFFFFFF;
}

s32 func_800BB764(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)g_field_script, 0x3F);
    return *g_field_script &= 0x7FFFFFFF;
}
