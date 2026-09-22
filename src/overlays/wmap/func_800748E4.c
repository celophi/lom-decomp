#include "wmap_sequence_runtime.h"
/* Partial WMAP decompilation: 88.090910% (gcc280_g0). */
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

extern WmapConfigA D_800D9580;
extern u8 *D_80139A18;
extern u16 D_8011CF4C[2];
extern s32 D_801B25E0;
extern s32 D_801B25B0;
extern s32 D_801B25B4;
extern void func_80066F9C(WmapConfigA *, s32, s32, s32, s32);

/** @brief Draw the actor, increase its scale, and advance when its timer expires. */
void func_800748E4(void)
{
    union
    {
        u16 half[2];
        s32 word;
    } position;

    position.half[0] = D_8011CF4C[0] + 24;
    position.half[1] = D_8011CF4C[1] + 4;
    D_800D9580.field_24 = *(u16 *)&D_801B25E0;
    D_800D9580.field_22 = *(u16 *)&D_801B25E0;
    func_8006CC4C(&D_800D9580, &D_80139A18);
    func_80066F9C(&D_800D9580, position.word, 4, 11, 0);
    D_801B25E0 += 8;
    if (D_801B25E0 >= 0x82)
    {
        D_801B25E0 = 0x81;
    }
    if (--D_801B25B4 == 0)
    {
        D_801B25B0++;
    }
}
