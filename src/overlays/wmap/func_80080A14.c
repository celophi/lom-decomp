#include "common.h"

extern u8 D_800D9478[];
extern u8 D_80139988[];
extern u8 D_8011F538[];
extern s32 D_801B27D0;
extern s32 D_801B27D4;
extern void func_80080AFC(void);

typedef struct
{
    s32 w[11];
} WmapBlk2C;

/**
 * @brief Populate a world-map actor pair and schedule its animation step.
 */
void func_80080A14(void)
{
    u8* src = D_800D9478;
    u8* dst = D_800D9478 + 0x2C;
    u8* tbl = D_80139988;

    *(u8**)&tbl[0x64] = D_8011F538;
    *(u8**)&tbl[0x6C] = D_8011F538;
    src[0x6] = 0xF;
    *(s16*)&src[0xE] = 2;
    *(s16*)&src[0x10] = -1;
    *(s16*)&src[0x26] = 8;
    *(s16*)&src[0x22] = 0x81;
    *(s16*)&src[0x2] = 0;
    *(s16*)&src[0x24] = 1;
    *(WmapBlk2C*)dst = *(WmapBlk2C*)src;
    *(s16*)&dst[0xE] = 3;
    D_801B27D4 = 0x2C;
    D_801B27D0 += 1;
    func_80080AFC();
}
