#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern u8 D_800D939C[];
extern u8 D_80182DE8[];
extern u8 D_801399C0[];
extern s32 D_8011CF4C;
extern s32 D_801B259C;
extern s32 D_801B2598;

/**
 * @brief Draw a scrolling world-map element and advance its slide/hold state.
 */
void func_80075ED0(void)
{
    u8* obj = D_800D939C;
    u16 pos = *(u16*)&D_80182DE8[0];

    *(s16*)&obj[0x24] = pos;
    *(s16*)&obj[0x22] = pos;
    func_8006CC4C(obj, D_801399C0);
    func_80066F9C(obj, D_8011CF4C, 0x4, 0xB, 0);
    *(s32*)&D_80182DE8[0] += 0x18;
    if (*(s32*)&D_80182DE8[0] >= 0x82)
    {
        *(s32*)&D_80182DE8[0] = 0x81;
    }
    if (--D_801B259C == 0)
    {
        D_801B2598 += 1;
    }
}
