#include "common.h"

/** @brief Screen coordinate scratch record retaining 16-bit components. */
typedef struct
{
    u16 x, y, z, pad;
} Point;
/** @brief Fixed-point offset used when projecting camera motion. */
typedef struct
{
    s32 x, y, z, pad;
} Vector;
/** @brief Scene identifier word followed by fixed-point camera coordinates. */
typedef struct
{
    s32 scene, x, y, z;
} Camera;
/** @brief Map dimensions used to clamp the camera. */
typedef struct
{
    u16 width, height;
} Bounds;
void func_80091BC8(void); /* extern */
void func_80092200(void); /* extern */
void func_800922B8(void); /* extern */
extern s32 D_800F2278;
extern s32 D_800F227C;
extern s32 D_800F2280;
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern Point D_801077FC;
extern s32 D_8010AE60;
extern s32 D_8010AE68;
extern s32 D_8010AE7C;
extern s32 D_8010AE80;
extern s32 D_8010D010;
extern s32 D_8010D014;

/**
 * @brief Update the camera, clamp it to map bounds, and record screen motion.
 *
 * Camera coordinates use eight fractional bits; projecting depth divides by
 * 512. The temporary points and zero offset preserve the original stack layout.
 */
void func_80091D7C(void)
{
    Point before;
    Point after;
    Vector offset;
    Camera *camera = (Camera *)0x801ED480;
    Bounds *bounds = (Bounds *)0x801ED400;
    s32 screen_x;
    s32 camera_screen_x, camera_screen_y, offset_screen_x, offset_screen_y;
    s32 screen_y;
    s32 follow_x;
    s32 follow_z;
    s32 clamp_x;
    s32 clamp_z;

    func_80091BC8();
    follow_x = D_8010D010;
    follow_z = D_8010D014;
    func_800922B8();
    func_80092200();
    offset.x = 0;
    offset.y = 0;
    offset.z = 0;
    before.x = D_800F22A0 / 256 + 160;
    before.y = D_800F22A4 / 256 + 112 - D_800F22A8 / 512;
    if (follow_x < (D_800F22A0 - 0x2000))
    {
        D_800F22A0 = follow_x + 0x2000;
    }
    if ((D_800F22A0 + 0x2000) < follow_x)
    {
        D_800F22A0 = follow_x - 0x2000;
    }
    if (follow_z < (D_800F22A8 - 0x2000))
    {
        D_800F22A8 = follow_z + 0x2000;
    }
    if ((D_800F22A8 + 0x2000) < follow_z)
    {
        D_800F22A8 = follow_z - 0x2000;
    }
    camera->x = D_800F22A0 + (D_800F2278 << 8) + 0xA000;
    D_800F22A4 = camera->y = D_800F227C << 8;
    camera->z = D_800F22A8 + (D_800F2280 << 9) + 0xE000;
    if (camera->x > -(D_8010AE60 << 8))
    {
        camera->x = -(D_800F2278 << 8) - (D_8010AE60 << 8);
        D_800F22A0 = camera->x - (D_800F2278 << 8) - 0xA000;
    }
    if (camera->x < -(D_8010AE68 << 8) + 0x14000)
    {
        clamp_x = (D_800F2278 << 8) - 0x14000;
        clamp_x = -(D_8010AE68 << 8) - clamp_x;
        camera->x = clamp_x;
        D_800F22A0 = camera->x - (D_800F2278 << 8) - 0xA000;
    }
    if (camera->z > 0)
    {
        camera->z = -(D_800F2280 << 9);
        D_800F22A8 = camera->z - (D_800F2280 << 9) - 0xE000;
    }
    if (camera->z < -((s32)(bounds->height << 16) >> 8) + 0x1C000)
    {
        clamp_z = (D_800F2280 << 9) - 0x1C000;
        clamp_z = -((s32)(bounds->height << 16) >> 8) - clamp_z;
        camera->z = clamp_z;
        D_800F22A8 = camera->z - (D_800F2280 << 9) - 0xE000;
    }
    if (D_8010AE7C != 0 || D_8010AE80 != 0)
    {
        camera->x = -(D_8010AE7C << 8);
        D_800F22A0 = -(D_8010AE7C + 0xA0) << 8;
        camera->z = -(D_8010AE80 << 9);
        D_800F22A8 = -(D_8010AE80 + 0x70) << 9;
    }
    camera_screen_x = D_800F22A0 / 256;
    offset_screen_x = offset.x / 256 + 160;
    screen_x = camera_screen_x + offset_screen_x;
    after.x = screen_x;
    camera_screen_y = D_800F22A4 / 256;
    offset_screen_y = offset.y / 256 + 112;
    screen_y = camera_screen_y + offset_screen_y - offset.z / 512 - D_800F22A8 / 512;
    after.y = screen_y;
    D_801077FC.x = screen_x - before.x;
    D_801077FC.y = screen_y - before.y;
}
