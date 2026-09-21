#include "common.h"

extern void func_8006D0F0(s32, s32 *, s32 *);
extern void func_800A89DC(s32);
extern void func_800A7400(void);
extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern s32 D_80139224;
extern s32 D_801398D0;
/** @brief Four-word world-map transform state. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    s32 pad;
} WmapTransform;

extern WmapTransform D_80139950;
extern s32 D_80139978;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_801B2E70;

/** @brief Set the map-relative effect position, load resources, and advance the sequence. */
void func_800A71D4(void)
{
    D_80139224 = 0;
    D_80139978 = 0x18;
    D_800DBE70 = 0;
    func_8006D0F0(0x18, &D_800DCEF8, &D_800DCF00);
    D_801398D0 = 2;
    D_8011D510 = D_800DCEF8;
    D_8011D530 = D_800DCF00;
    D_80182D68 = ((D_800DCEF8 - 1) * 0x30) - D_80139950.x;
    D_80182D78 = ((D_800DCF00 - 1) * 0x30) - D_80139950.y;
    func_800A89DC(0x21);
    D_801B2E70 += 1;
    func_800A7400();
}
