#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_80139888;
extern u16 D_8013B240;
extern s32 D_8011CF28;
extern s32 D_80182DE4;
extern s32 D_801B26A0;
extern s32 D_801B26A4;
extern void PushMatrix(void);
extern void PopMatrix(void);

/** @brief World-map animated element: advance phase, draw, and tick refcount. */
void func_80078F8C(void)
{
    s32 *p = &D_80139888;
    u16 *q;
    s32 v;
    s32 t;
    s32 c;

    v = p[2] - 0x5DC;
    p[2] = v;
    if (v <= 0x9C3F)
    {
        p[2] = 0x9C40;
    }
    PushMatrix();
    q = &D_8013B240;
    func_8006CFA8(p, q);
    func_8006CD98(D_8011CF28, 0, 0xC, 0x35, 0x7800, 1, D_80182DE4);
    t = D_80182DE4 + 0x10;
    D_80182DE4 = t;
    if (t >= 0x82)
    {
        D_80182DE4 = 0x81;
    }
    *(u16 *)((u8 *)q + 4) += 2;
    PopMatrix();
    c = D_801B26A4 - 1;
    D_801B26A4 = c;
    if (c == 0)
    {
        D_801B26A0 += 1;
    }
}
