#include "common.h"
#include "game_audio.h"
extern u32 *D_80123FB8;
s32 func_800B9BE0(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)D_80123FB8, 0x17);
    return *D_80123FB8 &= 0x7FFFFFFF;
}
