#include "common.h"

extern s32 D_800F2298;
extern u8 D_800EF85C[];
extern s16 D_8011F3D0;
extern u16 D_80122720;
extern u16 D_80122722;
extern s16 D_80122904;
extern s32 D_801229F0;

void field_load_vram_resource(s32 id, s16* rect, s32 arg2);
void field_set_fade_target_only(s16 red, s16 green, s16 blue, s16 duration);
void func_800A3938(s32 sound_id, s32 pan);

/**
 * @brief Initializes field resource state and selects its startup sound.
 * @param index Resource-table index used to select the field data and sound.
 */
void func_800A5670(s32 index)
{
    s16 rect[4];
    u8 value;
    s32 sound_id;

    if (D_800F2298 != 0)
    {
        return;
    }

    rect[0] = 0x140;
    rect[1] = 0;
    rect[2] = 0;
    rect[3] = 0x1F2;
    field_load_vram_resource(index + 0x1060, rect, 1);

    D_801229F0 = index;
    D_80122720 = rect[0];
    D_80122722 = rect[1];
    field_set_fade_target_only(0x80, 0x80, 0x80, 0x14);
    D_80122904 = 0x96;
    D_8011F3D0 = 0;
    D_800F2298 = 1;

    value = D_800EF85C[D_801229F0];
    if (value & 0x80)
    {
        switch (value & 0x7F)
        {
        case 0:
            sound_id = 0x11C;
            break;
        case 1:
            sound_id = 0x11A;
            break;
        case 2:
            sound_id = 0x11E;
            break;
        default:
            return;
        }
    }
    else
    {
        sound_id = 0x118;
    }

    func_800A3938(sound_id, 0x80);
}
