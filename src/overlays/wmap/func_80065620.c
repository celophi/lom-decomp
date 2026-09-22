#include "common.h"

/** @brief Packed color and GPU command byte. */
typedef union
{
    u32 packed;
    struct
    {
        u8 r, g, b, code;
    } channels;
} WmapColor;
extern void func_800667E8(u32);

extern WmapColor D_8011D50C;
extern WmapColor D_80129548;
extern s32 D_801398B0;
extern u8 D_8013B24C;

/** @brief Step the map tint toward its target color.
 * @return Nonzero if a color channel was adjusted.
 */
s32 func_80065620(void)
{
    s32 packed_color;
    s32 changed;

    changed = 0;
    if (D_8011D50C.channels.r > D_80129548.channels.r)
    {
        changed = 1;
        D_80129548.channels.r = (u8) (D_80129548.channels.r + D_8013B24C);
    }
    if ((u8) D_8011D50C.channels.r < (u8) D_80129548.channels.r)
    {
        changed = 1;
        D_80129548.channels.r = (u8) (D_80129548.channels.r - D_8013B24C);
    }
    if ((u8) D_80129548.channels.g < (u8) D_8011D50C.channels.g)
    {
        changed = 1;
        D_80129548.channels.g = (u8) (D_80129548.channels.g + D_8013B24C);
    }
    if ((u8) D_8011D50C.channels.g < (u8) D_80129548.channels.g)
    {
        changed = 1;
        D_80129548.channels.g = (u8) (D_80129548.channels.g - D_8013B24C);
    }
    if ((u8) D_80129548.channels.b < (u8) D_8011D50C.channels.b)
    {
        changed = 1;
        D_80129548.channels.b = (u8) (D_80129548.channels.b + D_8013B24C);
    }
    if ((u8) D_8011D50C.channels.b < (u8) D_80129548.channels.b)
    {
        changed = 1;
        D_80129548.channels.b = (u8) (D_80129548.channels.b - D_8013B24C);
    }
    if (D_801398B0 != 0)
    {
        packed_color = D_80129548.packed & 0xFFFFFF;
        if (packed_color == 0x808080)
        {
            D_8011D50C.packed = packed_color;
            D_80129548.packed = packed_color;
            D_80129548.channels.code = 0x2C;
            D_801398B0 = 0;
        }
        else
        {
            D_80129548.channels.code = 0x2E;
        }
    }
    else
    {
        D_80129548.channels.code = 0x2C;
    }
    func_800667E8(D_80129548.packed);
    return changed;
}
