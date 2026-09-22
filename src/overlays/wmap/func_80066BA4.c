/* Partial WMAP decompilation: 93.723404% (gcc280_g0). */
#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

typedef struct
{
    s16 x, y;
} WmapPoint;

typedef struct
{
    s32 x, y, scale, pad;
} WmapTransform;

extern WmapPoint D_80054944[];
extern WmapTransform D_80139950;
extern SVECTOR D_80139278;
extern VECTOR D_80182DC0;
extern s16 D_800D928A;
extern void func_8006D8F0(s32);
extern void func_8006D870(s32);
extern s32 D_800DBE70;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_801398D0;
extern s32 D_8013B20C;
extern s32 D_80182234;
extern s32 D_8018223C;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_801B1098;
extern s32 D_801B109C;

/** @brief Initialize the map effect and project its initial screen position. */
void func_80066BA4(void)
{
    SVECTOR position;
    MATRIX matrix;
    u32 screen;

    D_8013B20C = 1;
    func_8006D8F0(1);
    func_8006D870(1);
    D_801398D0 = 2;
    D_80182D68 = D_80054944[D_800DCEF0 * 3 + D_800DCEEC].x;
    D_80182D78 = D_80054944[D_800DCEF0 * 3 + D_800DCEEC].y;
    RotMatrix(&D_80139278, &matrix);
    TransMatrix(&matrix, &D_80182DC0);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    position.vz = 0;
    position.vx = ((D_80139950.x * 0x14000 / D_80139950.scale) * 0x6000) / D_80139950.scale;
    position.vy = ((D_80139950.y * 0x14000 / D_80139950.scale) * 0x6000) / D_80139950.scale;
    gte_ldv0(&position);
    gte_rtps();
    gte_stsxy(&screen);
    D_800DBE70 = 1;
    D_800D928A = 0;
    D_801B109C = 4;
    D_80182234 = (s16)screen;
    D_8018223C = (s16)(screen >> 16);
    D_801B1098++;
}
