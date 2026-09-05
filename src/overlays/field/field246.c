#include "common.h"
#include "game_audio.h"
extern u32 *g_field_script;
s32 func_800B9BE0(void)
{
    akao_set_song_params(0x8001, 1, *(u8 *)g_field_script, 0x17);
    return *g_field_script &= 0x7FFFFFFF;
}
