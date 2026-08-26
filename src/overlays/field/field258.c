#include "common.h"

extern void field_set_fade_target_only(s16 red, s16 green, s16 blue, s16 duration);
extern void func_800A3938(s32 sound_id, s32 pan);
extern void func_800AE9E0(void);

extern s32 D_8011F3D4;
extern s32 D_8012269C;
extern s32 D_80122990;
extern s32 D_80122B08;

void func_800AB710(void)
{
    D_8012269C = 7;
    field_set_fade_target_only(0xC0, 0xC0, 0xC0, 8);
    func_800AE9E0();
    func_800A3938(0x125, 0x80);
    D_8011F3D4 = 0;
    D_80122B08 = 0;
    D_80122990 = 0x1F4;
}
