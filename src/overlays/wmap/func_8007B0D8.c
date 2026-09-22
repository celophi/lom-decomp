#include "common.h"

extern u8 D_800DCF18[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern s32 D_80139234;
extern s32 D_8013923C;
extern void func_800675F0(u8 *, s32, s32, s32, s32, s32, s32, s32, s32, s32);

/** @brief Draw nine map effect sprites with fixed positions and ordering depths. */
void func_8007B0D8(s32 color_mask)
{
    func_800675F0(D_800DCF18, 0, 0x24, 0x35, 0x7800, 1, D_8013923C | color_mask, 0x46, -5, D_80139234);
    func_800675F0(D_8011CF1C, 0, 0x26, 0x35, 0x7800, 1, D_8013923C | color_mask, 0x64, -0x19, D_80139234);
    func_800675F0(D_8011CF24, 0, 0x22, 0x35, 0x7800, 1, D_8013923C | color_mask, 0x50, 0x1E, D_80139234);
    func_800675F0(D_800DCF18, 0, 0x20, 0x35, 0x7800, 1, D_8013923C | color_mask, 0x14, 0x28, D_80139234);
    func_800675F0(D_8011CF1C, 0, 0x1F, 0x35, 0x7800, 1, D_8013923C | color_mask, 0x32, 0x41, D_80139234);
    func_800675F0(D_8011CF24, 0, 0x21, 0x35, 0x7800, 1, D_8013923C | color_mask, -0x28, 0x23, D_80139234);
    func_800675F0(D_800DCF18, 0, 0x25, 0x35, 0x7800, 1, D_8013923C | color_mask, -0x28, -0x14, D_80139234);
    func_800675F0(D_8011CF1C, 0, 0x23, 0x35, 0x7800, 1, D_8013923C | color_mask, -0x44, 0xA, D_80139234);
    func_800675F0(D_8011CF24, 0, 0x27, 0x35, 0x7800, 1, D_8013923C | color_mask, -5, -0x32, D_80139234);
}
