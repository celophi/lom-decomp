#include "common.h"

/** @brief {min, span} threshold pair from the D_800FF610 table (stride 4). */
typedef struct
{
    u16 min;  /* 0x00 */
    u16 span; /* 0x02 */
} FieldThreshold;

/** @brief Camera / scroll state block at 0x801ED480 (see field_scene_internal.h). */
typedef struct
{
    u8 _pad[4];
    s32 x; /* 0x04 == g_field_camera_x */
    s32 y; /* 0x08 == g_field_camera_y */
    s32 z; /* 0x0C == g_field_camera_z */
} FieldCamera;

extern s32 D_800FE754;
extern s32 D_800FF650;
extern FieldThreshold D_800FF610[];
extern s32 D_8010AE58;
extern s32 D_8010AE6C;
extern s32 D_8010AE70;
extern s16 D_801ED400;

/**
 * @brief Pick the interpolation target/duration for the D_8010AE60-family
 *        camera-shake-or-scroll state, based on the current mode counter
 *        D_800FE754 and the active D_800FF610 threshold band.
 *
 * When D_800FE754 is 0, resets the state to seed from D_801ED400 (a 0x20
 * frame ramp). Otherwise, if the mode has already advanced past the
 * D_800FF610[D_800FE754 - 1] band's endpoint or its span is inside 0x140,
 * the target is derived from the current camera x scroll
 * (-(camera_x >> 8), plus a 0x140 frame ramp of 0x10 frames); otherwise the
 * target comes straight from the threshold entry's min/min+span (0x20 frame
 * ramp).
 *
 * @note 93.73% match. The remaining gap is one extra instruction: the target
 *       shares a single register for the D_8010AE58 address between the two
 *       tail paths (camera-adjust vs threshold band), while this compile
 *       allocates it to a different register in each path. Statement-order,
 *       declaration-order, and shared-address-local probes were all inert or
 *       regressed; sched_oracle reports no unsatisfied emit-order
 *       constraints, so this is a pure register-coloring difference, not a
 *       scheduling one.
 */
void func_80092124(void)
{
    s32 mode;
    FieldThreshold *b;
    FieldThreshold *entry;
    FieldCamera *cam;
    s32 idx;
    s32 val;

    cam = (FieldCamera *)0x801ED480;
    mode = D_800FE754;
    if (mode == 0)
    {
        D_8010AE6C = 0;
        D_8010AE58 = 0x20;
        D_8010AE70 = (s32) D_801ED400;
        return;
    }

    if (D_800FF650 < mode)
    {
        goto camera_path;
    }

    b = D_800FF610;
    idx = mode - 1;
    entry = &b[idx];
    if (entry->span < 0x140)
    {
camera_path:
        D_8010AE58 = 0x10;
        val = -(cam->x >> 8);
        D_8010AE6C = val;
        D_8010AE70 = val + 0x140;
        return;
    }

    D_8010AE58 = 0x20;
    D_8010AE6C = entry->min;
    D_8010AE70 = entry->min + entry->span;
}
