#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
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

extern WmapConfigA D_800D9554;
extern u8 *D_80139A10;
extern u16 D_8011CF4C[2];
extern s32 D_801B25DC;
extern s32 D_801B25A8;
extern s32 D_801B25AC;

/** @brief Draw the actor, reduce its scale, and advance when its timer expires. */
void func_80074820(void)
{
    union
    {
        u16 half[2];
        s32 word;
    } position;

    position.half[0] = D_8011CF4C[0] + 16;
    position.half[1] = D_8011CF4C[1];
    func_8006CC4C(&D_800D9554, &D_80139A10);
    func_80066F9C(&D_800D9554, position.word, 4, 11, 0);
    D_800D9554.field_24 = *(u16 *)&D_801B25DC;
    D_800D9554.field_22 = *(u16 *)&D_801B25DC;
    D_801B25DC -= 8;
    if (D_801B25DC < 0)
    {
        D_801B25DC = 0;
    }
    if (--D_801B25AC == 0)
    {
        D_801B25A8++;
    }
}
