#include "common.h"

extern unsigned char D_80117EF8;
extern s32 D_8011588C;
extern s32 D_8011F314;

void func_800A38D4(void)
{
    s32 temp_v0;

    temp_v0 = akao_cmd_19_c0((s32) &D_80117EF8, D_8011588C);
    D_8011F314 = temp_v0;
}

extern s32 D_8011F310;

void func_800A3904(s32 arg0)
{
    akao_cmd_c1((&D_8011F310)[arg0]);
}

void akao_play_sfx(s32, s32, s32, s32);

/**
 * @brief Play a FIELD sound effect at maximum volume.
 * @param sound_id Sound id forwarded to akao_play_sfx's arg0.
 * @param pan Forwarded to akao_play_sfx's arg2.
 */
void func_800A3938(s32 sound_id, s32 pan)
{
    akao_play_sfx(sound_id, 0, pan, 0x7F);
}

void akao_play_sfx(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

void func_800A3960(s32 arg0, s32 arg1)
{
    akao_play_sfx(arg0, 0, arg1 * 2, 0x7F);
}

extern void func_800A39A8(s32 sfx_index, s32 pan, s32 unused, s32 channel_group);

void func_800A3988(s32 sfx_index, s32 pan, s32 unused)
{
    func_800A39A8(sfx_index, pan, unused, 0);
}
