#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern u8 D_800D9370[];
extern u8 D_80182DE4[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;
extern s32 D_801B2594;
extern s32 D_801B2590;

/**
 * @brief Draw a scrolling world-map element and advance its slide/hold state.
 */
void func_80075C30(void)
{
    u8* obj = D_800D9370;
    u16 pos = *(u16*)&D_80182DE4[0];

    *(s16*)&obj[0x24] = pos;
    *(s16*)&obj[0x22] = pos;
    func_8006CC4C(obj, D_801399B8);
    func_80066F9C(obj, D_8011CF4C, 0xF, 0xB, 0);
    *(s32*)&D_80182DE4[0] += 4;
    if (*(s32*)&D_80182DE4[0] >= 0x82)
    {
        *(s32*)&D_80182DE4[0] = 0x81;
    }
    if (--D_801B2594 == 0)
    {
        D_801B2590 += 1;
    }
}
