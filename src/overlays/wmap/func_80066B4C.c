#include "common.h"

/** @brief World-map display record with a leading state field. */
typedef struct
{
    s16 state;
    u8 unknown_2[0x12];
} WmapDisplayState;

/** @brief Pointer slot in the world-map display table. */
typedef struct
{
    s32 unknown_0;
    u8 *data;
} WmapPointerSlot;

extern u8 D_8011D538[];
extern u8 D_80139988[];
extern WmapDisplayState D_801AFBD0[];
extern s32 D_801AFBC8;
extern s32 D_801B0FD0;

/** @brief Reset 110 display states, install their default pointers, and enable processing. */
void func_80066B4C(void)
{
    s32 index = 0;
    u8 *base = D_80139988;
    u8 *data = D_8011D538;
    s32 offset = 0x30;
    WmapPointerSlot *slot;
    do
    {
        slot = (WmapPointerSlot *)(offset + (s32)base);
        offset += 8;
        D_801AFBD0[index].state = 0;
        slot->data = data;
        index++;
    } while (index < 110);
    D_801B0FD0 = 1;
    D_801AFBC8 = 1;
}
