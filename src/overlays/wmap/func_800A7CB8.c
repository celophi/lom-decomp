#include "common.h"

extern u8 D_800D92EC[];
extern s32 D_801B2E48;
extern s32 D_801B2E4C;

/** @brief Reset a world-map HUD sprite record, then bump its shared refcount. */
void func_800A7CB8(void)
{
    D_800D92EC[0x6] = 0xF;
    *(s16*)&D_800D92EC[0x10] = -1;
    *(s16*)&D_800D92EC[0x22] = 0x80;
    *(s16*)&D_800D92EC[0x2] = 0;
    *(s16*)&D_800D92EC[0xE] = 0;
    *(s16*)&D_800D92EC[0x24] = 0;
    *(s16*)&D_800D92EC[0x26] = 8;
    D_801B2E4C = 0x10;
    D_801B2E48 += 1;
}
