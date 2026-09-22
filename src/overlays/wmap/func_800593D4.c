#include "wmap_main.h"
#include "wmap_effect_backdrop.h"
/* Partial WMAP decompilation: 82.769230% (gcc280_g0). */
#include "common.h"

/** @brief Signed map-screen coordinate pair. */
typedef struct
{
    s16 x;
    s16 y;
} WmapPoint;

extern s32 func_8005D4A4(void);
extern void func_80064F64(s32);
extern void func_8006683C(s32);
extern WmapPoint D_80054944[];
extern s16 D_800D928A;
extern s32 D_800DBE70;
extern s32 D_800DBE78;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_8011CF44;
extern s32 D_80139244;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern s32 D_8013B208;
extern s32 D_8013B254;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_80182E34;
extern s32 D_801ADAF4;

/** @brief Start the world-map transition when idle.
 * @return Zero when started, or one while another transition is active.
 */
s32 func_800593D4(void)

{
    WmapPoint *point;

    if (D_8011CF44 != 0)
    {
        return 1;
    }
    func_80064F64(func_8005D4A4() + 0x10CE);
    func_8006683C(0x808080);
    D_800DBE70 = 2;
    D_8013B254 = 1;
    D_800DBE78 = 2;
    D_80139244 = 0;
    D_80182E34 = 2;
    D_801ADAF4 = 0x10;
    func_8006D870(0);
    D_801398D0 = 2;
    D_8013986C = 0;
    D_8013B208 = 0;
    D_800D928A = 0x80;
    point = &D_80054944[D_800DCEF0 * 3 + D_800DCEEC];
    D_80182D68 = (s32) -point->x;
    D_80182D78 = (s32) -point->y;
    func_80064094();
    return 0;
}
