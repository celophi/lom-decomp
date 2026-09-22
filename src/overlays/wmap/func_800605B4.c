#include "common.h"
/** @brief Flat triangle polygon packet. */
typedef struct
{
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0, x1, y1, x2, y2;
} WmapTriangle;

extern WmapTriangle D_80051A1C;
extern WmapTriangle D_80051A30;
extern WmapTriangle D_80051A44;
extern WmapTriangle D_800DBE80;
extern s32 D_8011CF80;
extern WmapTriangle D_8011D518;
extern WmapTriangle D_801391E8;
extern WmapTriangle D_801398D8;
extern WmapTriangle D_80182DA0;
extern s8 D_80182E0C;
extern s32 D_801ADAE4;

/** @brief Restore map polygon templates and reset the display selection. */
void func_800605B4(void)
{
    D_8011D518 = D_80051A1C;
    D_80182DA0 = D_80051A44;
    D_801391E8 = D_80051A44;
    D_801398D8 = D_80051A30;
    D_800DBE80 = D_80051A30;
    D_801391E8.b0 = 0;
    D_801391E8.g0 = 0;
    D_801391E8.r0 = 0;
    D_800DBE80.b0 = 0;
    D_800DBE80.g0 = 0;
    D_800DBE80.r0 = 0;
    D_80182E0C = 0;
    D_8011D518.r0 = 0;
    D_801ADAE4 = -1;
    D_8011CF80 = -1;
}
