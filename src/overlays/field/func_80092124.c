#include "common.h"

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
 * @brief Select the interpolation bounds and duration for the current field camera mode.
 */
void func_80092124(void)
{
    s32 mode;
    FieldThreshold* thresholds;
    FieldThreshold* entry;
    FieldCamera* camera;
    s32 index;
    s32 value;

    camera = (FieldCamera*)0x801ED480;
    mode = D_800FE754;
    if (mode == 0)
    {
        D_8010AE6C = 0;
        D_8010AE58 = 0x20;
        D_8010AE70 = *(s16*)0x801ED400;
        return;
    }

    if (D_800FF650 < mode)
    {
        goto camera_path;
    }

    thresholds = D_800FF610;
    index = mode - 1;
    entry = &thresholds[index];
    if (entry->span < 0x140)
    {
camera_path:
        D_8010AE58 = 0x10;
        value = D_8010AE6C = -(camera->x >> 8);
        D_8010AE70 = value + 0x140;
        return;
    }

    D_8010AE58 = 0x20;
    D_8010AE6C = entry->min;
    D_8010AE70 = entry->min + entry->span;
}
