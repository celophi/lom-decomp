#include "common.h"
#include "display.h"

typedef struct
{
    u16 min;
    u16 span;
} FieldThreshold;

typedef struct
{
    u8 _pad[4];
    s32 x;
    s32 y;
    s32 z;
} FieldCamera;

extern s32 D_800FE754;
extern s32 D_800FF650;
extern FieldThreshold D_800FF610[];
extern s32 D_8010AE58;
extern s32 D_8010AE6C;
extern s32 D_8010AE70;

/**
 * @brief Configure horizontal camera interpolation bounds for the active field camera mode.
 */
void func_80092124(void)
{
    s32 camera_mode;
    FieldThreshold* camera_thresholds;
    FieldThreshold* threshold;
    FieldCamera* camera;
    s32 threshold_index;
    s32 lower_bound;

    camera = (FieldCamera*)0x801ED480;
    camera_mode = D_800FE754;
    if (camera_mode == 0)
    {
        D_8010AE6C = 0;
        D_8010AE58 = 32;
        D_8010AE70 = *(s16*)0x801ED400;
        return;
    }

    if (D_800FF650 < camera_mode)
    {
        goto camera_bounds;
    }

    camera_thresholds = D_800FF610;
    threshold_index = camera_mode - 1;
    threshold = &camera_thresholds[threshold_index];
    if (threshold->span < SCREEN_WIDTH)
    {
camera_bounds:
        D_8010AE58 = 16;
        lower_bound = D_8010AE6C = -(camera->x >> 8);
        D_8010AE70 = lower_bound + SCREEN_WIDTH;
        return;
    }

    D_8010AE58 = 32;
    D_8010AE6C = threshold->min;
    D_8010AE70 = threshold->min + threshold->span;
}
