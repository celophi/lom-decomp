#include "wmap_sequence_runtime.h"
#include "common.h"

/** @brief Four-word world-map projection state. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    s32 pad;
} WmapTransform;

extern u8 D_800DEF18[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern s16 D_8011CF4C[];
extern s32 D_8013B258;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_801398D0;
extern WmapTransform D_80139950;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_801B2E78;
extern void func_800AB8E0(void);

/** @brief Set the effect resources and map-relative position, then advance. */
void func_800AAA2C(void)
{
    D_8011CF1C = D_800DEF18;
    D_8011CF24 = D_800DEF18 + 0x2000;
    D_8011CF4C[0] = 0x94;
    D_8011CF4C[1] = 0x31;
    D_8013B258 = 1;
    func_8006D0F0(0x11, &D_800DCEF8, &D_800DCF00);
    D_801398D0 = 2;
    D_80182D68 = ((D_800DCEF8 - 1) * 0x30) - D_80139950.x;
    D_80182D78 = ((D_800DCF00 - 1) * 0x30) - D_80139950.y;
    D_801B2E78++;
    func_800AB8E0();
}
