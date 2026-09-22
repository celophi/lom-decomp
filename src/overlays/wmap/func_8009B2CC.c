#include "common.h"
#include "sdk/libgte.h"

extern s32 D_800D665C[];
extern void *D_8011CF24;
extern void *D_8011CF28;
extern void *D_8011CF2C;
extern s32 D_80139234;
extern SVECTOR D_8013B238;
extern VECTOR D_80182DC0;
extern s32 D_80182DF0;
extern SVECTOR D_801B24A8;
extern s32 D_801B2C90;
extern s32 D_801B2C94;
extern void func_8006CFA8(VECTOR *, SVECTOR *);
extern void func_800675F0(void *, s32, s32, s32, s32, s32, s32, s32, s32, s32);

/** @brief Draw three rotating effect layers and update their fade and animation. */
void func_8009B2CC(void)
{
    s32 intensity;
    s32 frame;
    s32 remaining;
    u16 angle;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_8013B238);
    func_800675F0(D_8011CF24, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -10, -1);
    func_8006CFA8(&D_80182DC0, &D_801B24A8);
    func_800675F0(D_8011CF28, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -40, -1);
    func_8006CFA8(&D_80182DC0, &D_8013B238);
    func_800675F0(D_8011CF2C, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -70, -1);
    PopMatrix();
    intensity = D_80182DF0 - 4;
    D_80182DF0 = intensity;
    if (intensity < 0)
    {
        D_80182DF0 = 0;
    }
    angle = D_8013B238.vz;
    D_8013B238.vz = angle + 330;
    D_801B24A8.vz += 10;
    D_8013B238.vz = angle + 340;
    frame = D_80139234 + 1;
    D_80139234 = frame;
    if (frame >= 6)
    {
        D_80139234 = 0;
    }
    remaining = D_801B2C94 - 1;
    D_801B2C94 = remaining;
    if (remaining == 0)
    {
        D_801B2C90++;
    }
}
