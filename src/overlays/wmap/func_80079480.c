#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2660;
extern u16 D_801B2678;
extern s32 D_8011CF30;
extern s32 D_80182DEC;
extern s32 D_801B26B0;
extern s32 D_801B26B4;
extern void PushMatrix(void);
extern void PopMatrix(void);

/** @brief World-map animated element: advance phase, draw while active, then tick refcount. */
void func_80079480(void)
{
    s32 *p = &D_801B2660;
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
    q = &D_801B2678;
    func_8006CFA8(p, q);
    if (D_80182DEC != 0)
    {
        func_8006CD98(D_8011CF30, 0, 0xC, 0x35, 0x7800, 1, D_80182DEC);
        t = D_80182DEC - 2;
        D_80182DEC = t;
        if (t < 0)
        {
            D_80182DEC = 0;
        }
        *(u16 *)((u8 *)q + 4) += 4;
    }
    PopMatrix();
    c = D_801B26B4 - 1;
    D_801B26B4 = c;
    if (c == 0)
    {
        D_801B26B0 += 1;
    }
}
