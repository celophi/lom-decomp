#include "wmap_resource_support.h"
#include "common.h"

/** @brief Three color channels. */
typedef struct
{
    u8 r, g, b;
} WmapColor;

extern s32 D_8013B208;
extern WmapColor D_80182D74;
extern WmapColor D_80182D80;
extern WmapColor D_80182D8C;
extern WmapColor D_80182D94;
extern s32 D_801B2F98;
extern s32 D_801B2F9C;

/** @brief Set the sequence colors and start its fifteen-frame countdown. */
void func_800B22D4(void)
{
    D_8013B208 = 1;
    func_800652A8(0x37, 0x80);
    D_80182D74.r = 0x32;
    D_80182D74.g = 0;
    D_80182D74.b = 0xA0;
    D_80182D80.r = 0x32;
    D_80182D80.g = 0;
    D_80182D80.b = 0xA0;
    D_80182D8C.r = 0;
    D_80182D8C.g = 0;
    D_80182D8C.b = 0;
    D_80182D94.r = 0;
    D_80182D94.g = 0;
    D_80182D94.b = 0;
    D_801B2F9C = 0xF;
    D_801B2F98 += 1;
}
