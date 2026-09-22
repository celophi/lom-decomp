#include "common.h"

extern u8 D_800D9420[];
extern u8 D_80139988[];
extern u8 D_8011F538[];
extern s32 D_801B27C8;
extern s32 D_801B27CC;
extern void func_80080888(void);

typedef struct
{
    s32 w[11];
} WmapBlk2C;

/**
 * @brief Populate a world-map actor pair and schedule its animation step.
 */
void func_800807A0(void)
{
    u8* src = D_800D9420;
    u8* dst = D_800D9420 + 0x2C;
    u8* tbl = D_80139988;

    *(u8**)&tbl[0x54] = D_8011F538;
    *(u8**)&tbl[0x5C] = D_8011F538;
    src[0x6] = 0xF;
    *(s16*)&src[0xE] = 2;
    *(s16*)&src[0x10] = -1;
    *(s16*)&src[0x26] = 8;
    *(s16*)&src[0x22] = 0x81;
    *(s16*)&src[0x2] = 0;
    *(s16*)&src[0x24] = 1;
    *(WmapBlk2C*)dst = *(WmapBlk2C*)src;
    *(s16*)&dst[0xE] = 3;
    D_801B27CC = 0x40;
    D_801B27C8 += 1;
    func_80080888();
}
