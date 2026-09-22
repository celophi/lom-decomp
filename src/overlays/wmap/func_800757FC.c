#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
/* Partial WMAP decompilation: 92.652176% (gcc280_g0). */
#include "common.h"

/** @brief World-map actor configuration. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[2];
    u8 field_06;
    u8 pad_07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapConfigA;

extern WmapConfigA D_800D9318;
extern u8 *D_801399A8;
extern s32 D_8011CF4C;
extern s32 D_801B24B4;
extern s32 D_801B2578;
extern s32 D_801B257C;

/** @brief Draw the actor, increase its scale, and advance when its timer expires. */
void func_800757FC(void)
{

    D_800D9318.field_24 = *(u16 *)&D_801B24B4;
    D_800D9318.field_22 = *(u16 *)&D_801B24B4;
    func_8006CC4C(&D_800D9318, &D_801399A8);
    func_80066F9C(&D_800D9318, D_8011CF4C, 15, 4, 0);
    D_801B24B4 += 8;
    if (D_801B24B4 >= 0x82)
    {
        D_801B24B4 = 0x81;
    }
    if (--D_801B257C == 0)
    {
        D_801B2578++;
    }
}
