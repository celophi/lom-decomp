#include "wmap_sequence_runtime.h"
#include "common.h"
#include "sdk/libgte.h"

extern void *D_8011CF1C;
extern s32 D_801B2474;
extern VECTOR D_801B2478;
extern SVECTOR D_801B24A8;
extern s32 D_801B2600;
extern s32 D_801B2604;
extern void func_800675F0(void *, s32, s32, s32, s32, s32, s32, s32, s32, s32);

/** @brief Move the effect toward the camera while fading it and advancing its countdown. */
void func_80076DBC(void)
{
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = D_801B2478.vz - 2500;
    D_801B2478.vz = depth;
    if (depth < 100)
    {
        D_801B2478.vz = 100;
    }
    PushMatrix();
    func_8006CFA8(&D_801B2478, &D_801B24A8);
    if (D_801B2474 != 0)
    {
        func_800675F0(D_8011CF1C, 0, 4, -1, -1, 1, D_801B2474, 5, -20, -1);
    }
    PopMatrix();
    intensity = D_801B2474 - 8;
    D_801B2474 = intensity;
    if (intensity < 0)
    {
        D_801B2474 = 0;
    }
    remaining = D_801B2604 - 1;
    D_801B2604 = remaining;
    if (remaining == 0)
    {
        D_801B2600++;
    }
}
